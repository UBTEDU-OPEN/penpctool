#include "aipenserver.h"
#include "logHelper.h"
#include "projectconst.h"
#include "config.h"
#include "aipenmanager.h"
#include "tqlsdkadapter.h"
#include "getportuitls.h"
#include "localxmlworker.h"

#include <QDataStream>
#include "ubtserver.h"

#include "alyservice.h"
#include "qt_windows.h"

#include <string>

using namespace std;

using namespace Json;

#ifdef SHOW_TEST_BTN
#include <QFileInfo>
#include "postdotthread.h"
PostDotThread* postDotThread1 = nullptr;
#endif

AiPenServer::AiPenServer():
    m_pConnection(nullptr)
  , httpWorker_(nullptr)
{

//    HRESULT e = OleInitialize(0);
    httpWorker_ = new HttpWorker;
    httpWorker_->moveToThread(&httpWorkThread_);

    UBTServer* ubtInst = UBTServer::instance();
    ubtInst->moveToThread(&httpWorkThread_);

    connect(this,SIGNAL(sms(QString)),this,SLOT(readSMS(QString)));

    if(m_server.listen(AIPEN_SERVER_NAME))
    {
        //Catch all new connections
        connect(&m_server, SIGNAL(newConnection()), this, SLOT(newConnection()));
    }

    connect(AiPenManager::getInstance(),&AiPenManager::httpRequest,httpWorker_,&HttpWorker::onHttpRequest);
    connect(httpWorker_,&HttpWorker::postResult,AiPenManager::getInstance(),&AiPenManager::postResult);
    connect(AiPenManager::getInstance(),&AiPenManager::updateWordXml,LocalXmlWorker::instance(),&LocalXmlWorker::updateWordXml);
    connect(LocalXmlWorker::instance(),&LocalXmlWorker::httpRequest,httpWorker_,&HttpWorker::onHttpRequest);
    connect(httpWorker_,&HttpWorker::sideXmlResponse,LocalXmlWorker::instance(),&LocalXmlWorker::onSideXmlResponse);
    //XML
    connect(UBTServer::instance(),&UBTServer::xmlDownloadFinished,LocalXmlWorker::instance(),&LocalXmlWorker::onDownloadFinished);
    connect(httpWorker_,&HttpWorker::sigGetBookDetail,AiPenManager::getInstance(),&AiPenManager::onGetBookDetail);
    connect(httpWorker_,&HttpWorker::sigGetWorkClassId,AiPenManager::getInstance(),&AiPenManager::onGetWorkClassId);


    connect(this,&AiPenServer::stopTimer,httpWorker_,&HttpWorker::onStopTimer);
    connect(ubtInst,&UBTServer::replyResult,httpWorker_,&HttpWorker::onReplyResult);

    m_msg_id = 1;
    //====阿里云初始化==start==
    auto alyInst = AlyService::instance();
    alyInst->moveToThread(&httpWorkThread_);
    connect(alyInst,&AlyService::httpRequest,httpWorker_,&HttpWorker::onHttpRequest);
    AlyService::instance()->init();
    //====阿里云初始化==end==

    int port = 0;
    Config::getPort(port);
    if(TQLSDKAdapter::getInstance()->init(port,1) == 0) {
        LOG(INFO)<<"TQLSDK init fial, port="<<port;
    }
    LOG(INFO)<<"TQLSDK init success, port="<<port;
    connect(TQLSDKAdapter::getInstance(), &TQLSDKAdapter::sigPenPower, this, &AiPenServer::onPenPower);
    connect(TQLSDKAdapter::getInstance(), &TQLSDKAdapter::sigPenFirmwareVersion, this, &AiPenServer::onPenFirmwareVersion);
    httpWorkThread_.start();

    //启动后先获取一下token
    bool bAutoLogin = false;
    qint64 userId = 0;
    QString token;
    QString loginId;
    Config::getLoginInfo(userId,token,loginId, bAutoLogin);
    LOG(INFO) << "AiPenServer::AiPenServer token=" << token.toStdString();
    UBTServer::instance()->onSaveToken(token);
}

AiPenServer::~AiPenServer()
{
    AiPenManager::getInstance()->stopClassOnExit();

    emit stopTimer();
    disconnect(AiPenManager::getInstance(),&AiPenManager::httpRequest,httpWorker_,&HttpWorker::onHttpRequest);
    disconnect(AlyService::instance(),&AlyService::httpRequest,httpWorker_,&HttpWorker::onHttpRequest);
    TQLSDKAdapter::getInstance()->doCloseReceiveDot();
    delete TQLSDKAdapter::getInstance();
    delete AiPenManager::getInstance();
    delete AlyService::instance();
    if (m_pConnection)
    {
        m_pConnection->close();
        m_pConnection->deleteLater();
    }
    httpWorkThread_.quit();
    httpWorkThread_.wait();

}

void AiPenServer::onPenPower(QString mac, int Battery, int Charging)
{
    send(buildPowerMsg(mac, Battery, Charging));
}

void AiPenServer::onPenFirmwareVersion(QString mac, QString FN)
{
    send(buildFirmwareVersionMsg(mac, FN));
}

void AiPenServer::newConnection()
{
    while (m_server.hasPendingConnections())
    {
        if (m_pConnection)
        {
            //清空旧连接
            disconnect(m_pConnection, SIGNAL(readyRead()), this , SLOT(readSocket()));
            disconnect(m_pConnection, SIGNAL(disconnected()), this , SLOT(discardSocket()));
            delete m_pConnection;
            m_pConnection = nullptr;
        }
        m_pConnection = m_server.nextPendingConnection();//更新连接，只允许连接一个
        connect(m_pConnection, SIGNAL(readyRead()), this , SLOT(readSocket()));
        connect(m_pConnection, SIGNAL(disconnected()), this , SLOT(discardSocket()));
    }
}

QString AiPenServer::buildPowerMsg(QString mac, int Battery, int Charging)
{
    StreamWriterBuilder wBuilder;
    Value root;
    root["msg_id"] = m_msg_id++;
    root["command"] = ePro_PowerMsg;
    Value data;
    data["mac"] = mac.toStdString();
    data["power"] = Battery;
    data["charging"] = Charging;
    root["data"] = data;
    return QString(writeString(wBuilder, root).c_str());
}

QString AiPenServer::buildFirmwareVersionMsg(QString mac, QString FN)
{
    StreamWriterBuilder wBuilder;
    Value root;
    root["msg_id"] = m_msg_id++;
    root["command"] = ePro_FirmwareVersion;
    Value data;
    data["mac"] = mac.toStdString();
    data["FN"] = FN.toStdString();
    root["data"] = data;
    return QString(writeString(wBuilder, root).c_str());
}

void AiPenServer::HandleStartClasswork(const Json::Value &value)
{
    Classwork* work = new Classwork();
    work->workClassId = value["body"]["workClassId"].asInt();
    work->endTime = value["body"]["endTime"].asLargestUInt();
    work->lessonId = value["body"]["lessonId"].asInt();
    work->practiseType = value["body"]["practiseType"].asInt();
    work->practiseBriefInfo = QString::fromStdString(value["body"]["practiseBriefInfo"].asCString());
    work->practices.clear();
    QDateTime time = QDateTime::currentDateTime();//获取当前时间
    work->startTime = time.toMSecsSinceEpoch();
    Value rawWordsList = value["body"]["words"];
    int rowIndex = 0;
    Config::saveClassWorkInfo(true, work->workClassId);
    for (unsigned int i = 0; i < rawWordsList.size(); ++i)
    {
        //todo
        Value word = rawWordsList[i];
        Practice p;
        p.times = word["times"].asLargestUInt();
        p.contentCode = word["contentCode"].asInt();
        QString contentName = QString::fromStdString(word["contentName"].asCString());
        auto name = QByteArray::fromPercentEncoding(contentName.toLatin1());
        p.contentName = QString::fromUtf8(name);

        int wordOrder = -1;
        if(word.isMember("wordOrder")) {
            wordOrder = word["wordOrder"].asInt();
        }
        int row = -1;
        if(word.isMember("row")) {
            row = word["row"].asInt();
        }

        if(-1 != wordOrder && -1 == row) { //有wordOrder，但是row还没添加
            if (0 == wordOrder) {
                ++rowIndex; //row从1开始，不同于wordOrder
            }
            row = rowIndex;
        }

        if(-1 != wordOrder && -1 != row) {
            p.wordOrder = row * 10000 + wordOrder; //跟APP端一致
        } else {
            p.wordOrder = -1; //无效标记
        }

        if(0 == p.contentCode) {
            p.contentCode = -1;
        } else {
            p.contentId = word["contentId"].asInt();
            p.structureId = word["structureId"].asInt();
            p.characterStrokesNum = word["characterStrokesNum"].asInt();
            p.characterXmlUrl = QString::fromStdString(word["characterXmlUrl"].asCString());
            if(word.isMember("characterXmlUpdateTime")) {
                p.characterXmlUpdateTime = QString::fromStdString(word["characterXmlUpdateTime"].asCString());
            }
        }
        work->practices.push_back(p);
    }
    AiPenManager::getInstance()->onNotifyStartClasswork(work);
}


void AiPenServer::readSocket()
{
    //Pointer to the signal sender
    LOG(INFO) <<"AiPenServer::readSocket begin";
    QLocalSocket * socket = (QLocalSocket*)sender();

    //Read all data on the socket & store it on a QByteArray
    QByteArray block = socket->readAll();

    //Data stream to easy read all data
    QDataStream in(&block, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_4_0);

    while (!in.atEnd()) //loop needed cause some messages can come on a single packet
    {
        QString receiveString;
        in >> receiveString;
        emit sms(receiveString);
    }
    LOG(INFO) <<"AiPenServer::readSocket end";
}

void AiPenServer::discardSocket()
{
//    //Pointer to the disconnecting socket
//    QLocalSocket * ptrSender = (QLocalSocket*)sender();
//    //Delete the socket a soon as possible
//    ptrSender->deleteLater();
    m_pConnection->deleteLater();
    m_pConnection = nullptr;

}

void AiPenServer::readSMS(const QString msg)
{
//    QString org_msg;
//    QDataStream in(&msg.toUtf8(), QIODevice::ReadOnly);
//    in.setVersion(QDataStream::Qt_4_0);
//    in >> org_msg;

    LOG(INFO)<<"AiPenServer readSMS msg="<< msg.toStdString();
//    LOG(INFO)<<"AiPenServer readSMS org_msg="<< org_msg.toStdString();
    string stdJsonStr = msg.toStdString();
    istringstream iss(stdJsonStr);
    CharReaderBuilder rBuilder;
    Value root;
    String err;
    bool ok = parseFromStream(rBuilder, iss, &root, &err);
    if (ok)
    {
        int nCommond = root["command"].asInt();
        LOG(INFO)<<"AiPenServer readSMS nCommond="<< nCommond;
        switch (nCommond) {
        case ePro_AttendClass:
        {
            Value data = root["data"];
            int nClassId = data["classId"].asInt();
            AiPenManager::getInstance()->onNotifyAttendClass(nClassId);
            Config::saveClassInfo(true, nClassId);
        }
            break;
        case ePro_FinishClass:
        {
            AiPenManager::getInstance()->onNotifyFinishClass();
            Config::saveClassInfo(false, 0);
        }
            break;
        case ePro_FinishAllClass:
        {
            LocalXmlWorker::instance()->updateSideXml();
            bool bAutoLogin = false;
            qint64 userId = 0;
            QString token;
            QString loginId;
            Config::getLoginInfo(userId,token,loginId, bAutoLogin);
            UBTServer::instance()->onSaveToken(token);
            AiPenManager::getInstance()->finishAllClassWork();
            //test
//            onPenFirmwareVersion("sss", "sd");
        }
            break;
        case ePro_StartClasswork:
        {
            Value data = root["data"];
            HandleStartClasswork(data);
        }
            break;
        case ePro_StopClasswork:
        {
            Config::saveClassWorkInfo(false, 0);
            AiPenManager::getInstance()->onNotifyStopClasswork();
        }
            break;
        case ePro_StartDictation:
        {
            Value data = root["data"];
            int workClassId = data["workClassId"].asInt();
            AiPenManager::getInstance()->onNotifyStartDictation(workClassId);
        }
            break;
        case ePro_StopDictation:
        {
            AiPenManager::getInstance()->onNotifyStopDictation();
        }
            break;
        case ePro_DictationWord:
        {
            Value data = root["data"];
            int groupId = data["groupId"].asInt();
            QString contentCode = data["contentCode"].asCString();
            AiPenManager::getInstance()->onDictationWordStart(groupId,contentCode);
        }
            break;
        case ePro_NoFn:
        {
            Value data = root["data"];
            QString mac = data["mac"].asCString();
            QString ip = data["ip"].asCString();
            char apIp[100]{0};
            memcpy_s(apIp,ip.size(),ip.toStdString().c_str(),ip.size());
            char strMac[100]{0};
            memcpy_s(strMac,mac.size(),mac.toStdString().c_str(),mac.size());
            LOG(INFO) << "doGetPenFirmwareVersion:" << apIp << "," << strMac;
            TQLSDKAdapter::getInstance()->doGetPenFirmwareVersion(apIp,strMac);
        }
            break;
#ifdef SHOW_TEST_BTN
        case ePro_OpenOriginFile:
        {
            QString filePathes = root["data"].asCString();
            onOpenOriginFile(filePathes);
        }
            break;
#endif
        default:
            break;
        }
    }
}

void AiPenServer::send(const QString &msg)
{
    if(m_pConnection)
    {
        if(m_pConnection->isOpen())
        {
            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_0);
            out << msg;
            m_mutex.lock();
            m_pConnection->write(block);
            m_mutex.unlock();
        }
    }
}

#ifdef SHOW_TEST_BTN
void AiPenServer::onOpenOriginFile(QString filePathes)
{
    if(postDotThread1 != nullptr) {
        postDotThread1->stopRunning();
        postDotThread1->wait();
        delete postDotThread1;
    }

    postDotThread1 = new PostDotThread;

//    if(postDotThread2 != nullptr) {
//        postDotThread2->stopRunning();
//        postDotThread2->wait();
//        delete postDotThread2;
//    }

//    postDotThread2 = new PostDotThread;

    auto fileList = filePathes.split(";");

    int i = 0;
    for(auto filePath : fileList) {
        int j = i%2;
        QFileInfo fileInfo(filePath);
        QString baseName = fileInfo.baseName();
        QFile *file = new QFile(filePath);
        if(file->open(QIODevice::ReadOnly | QIODevice::Text)) {
//            switch (j) {
//            case 0:
                postDotThread1->pens.insert(baseName,file);
//                break;
//            case 1:
//                postDotThread2->pens.insert(baseName,file);
//                break;
//            default:
//                break;
//            }

        }
        ++i;
    }

    postDotThread1->start();
//    postDotThread2->start();
}
#endif
