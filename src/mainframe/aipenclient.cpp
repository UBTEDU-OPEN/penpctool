#include "aipenclient.h"
#include "QDataStream"
#include "logHelper.h"
#include "IPCProtocol.h"
#include "apmanager.h"

#include <QThread>

using namespace Json;
using namespace std;

AiPenLocalClient::AiPenLocalClient()
    :m_bCon(false)
{
    qDebug() << "AiPenLocalClient threadid" << QThread::currentThreadId();
    conection = new QLocalSocket(this);
    connect(conection,SIGNAL(readyRead()),this,SLOT(readSocket()));
    connect(conection,SIGNAL(connected()),this,SLOT(onConnectSocket()));
    connect(conection,SIGNAL(disconnected()),this,SLOT(discardSocket()));
    connect(this,SIGNAL(sms(QString)),this,SLOT(readSMS(QString)));
//    conection->connectToServer(AIPEN_SERVER_NAME);//启动的时候不连接
    m_msg_id = 1;
    connectTimer_ = new QTimer(this);
    connect(connectTimer_,&QTimer::timeout,this,&AiPenLocalClient::onTimeout);
    connectTimer_->start(1000);
}

AiPenLocalClient::~AiPenLocalClient()
{
    if(conection)
        conection->close();
}

void AiPenLocalClient::finishAllClassWork()
{
    bool ret = send(buildBaseMsg(ePro_FinishAllClass));
    if(!ret) {
        finishAllPending_ = true;
    }
}

void AiPenLocalClient::onNotifyAttendClass(int classId)
{
    send(buildAttendClass(classId));
}

void AiPenLocalClient::onNotifyFinishClass()
{
    send(buildBaseMsg(ePro_FinishClass));
}

void AiPenLocalClient::onNotifyStartClasswork(QString strClasswork)
{
    string stdJsonStr = strClasswork.toStdString();
    istringstream iss(stdJsonStr);
    CharReaderBuilder rBuilder;
    Value root;
    String err;
    bool ok = parseFromStream(rBuilder, iss, &root, &err);
    if (ok)
    {
        send(buildStartClasswork(root));
    }

}

void AiPenLocalClient::onNotifyStopClasswork()
{
    send(buildBaseMsg(ePro_StopClasswork));
}

void AiPenLocalClient::onNotifyStartDictation(int workClassId)
{
    send(buildStartDictation(workClassId));
}

void AiPenLocalClient::onNotifyStopDictation()
{
    send(buildBaseMsg(ePro_StopDictation));
}

void AiPenLocalClient::onDictationWordStart(QString msg)
{
    send(buildDictationWord(msg));
}

void AiPenLocalClient::onTimeout()
{
    LOG(INFO) << "AiPenLocalClient::onTimeout";
    qDebug() << "onTimeout threadid" << QThread::currentThreadId();
    if(!m_bCon) {
        conection->connectToServer(AIPEN_SERVER_NAME);
    }
}

void AiPenLocalClient::onNoFN(QString mac, QString ip)
{
    StreamWriterBuilder wBuilder;
    Value root;
    root["msg_id"] = m_msg_id++;
    root["command"] = ePro_NoFn;
    Value data;
    data["mac"] = mac.toStdString();
    data["ip"] = ip.toStdString();
    root["data"] = data;
    QString strWorkJson = QString(writeString(wBuilder, root).c_str());
    LOG(INFO) << "onNoFN: " << strWorkJson.toStdString();
    send(strWorkJson);
}

void AiPenLocalClient::readSocket()
{
    QByteArray block = conection->readAll();
    QDataStream in(&block, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_4_0);
    while (!in.atEnd())
    {
        QString receiveString;
        in >> receiveString;
//        receiveString.prepend(QString("%1 :: ").arg(conection->socketDescriptor()));
        emit sms(receiveString);
    }
}

void AiPenLocalClient::readSMS(QString sms)
{
    LOG(INFO) << "AiPenLocalClient:readSMS:" << sms.toStdString();
    string stdJsonStr = sms.toStdString();
    istringstream iss(stdJsonStr);
    CharReaderBuilder rBuilder;
    Value root;
    String err;
    bool ok = parseFromStream(rBuilder, iss, &root, &err);
    if (ok)
    {
        const int msg_id  = root["msg_id"].asInt();
        const int nCmd  = root["command"].asInt();
        switch (nCmd) {
        case ePro_PowerMsg:
        {
            Value data = root["data"];
            int nCharging = data["charging"].asInt();
            int nPower = data["power"].asInt();
            QString strMac = data["mac"].asCString();
            APManager::getInstance()->handlePenPower(strMac, nPower, nCharging);

        }
            break;
        case ePro_FirmwareVersion:
        {
             Value data = root["data"];
             QString strMac = data["mac"].asCString();
             QString strFn = data["FN"].asCString();
             APManager::getInstance()->handlePenFirmwareVersion(strMac, strFn);

        }
            break;
        default:
            break;
        }
    }
}

void AiPenLocalClient::onConnectSocket()
{
    m_bCon = true;
    LOG(INFO)<<"AiPenLocalClient onConnectSocket";
    if(nullptr != connectTimer_) {
        connectTimer_->stop();
        connectTimer_->deleteLater();
        connectTimer_ = nullptr;
    }
    if(finishAllPending_) {
        finishAllPending_ = false;
        finishAllClassWork();
    }
}

void AiPenLocalClient::discardSocket()
{
    //断开重连不清资源
//    conection->deleteLater();
//    conection = 0;
    m_bCon = false;
    LOG(INFO)<<"AiPenLocalClient discardSocket";
}

QString AiPenLocalClient::buildBaseMsg(int nCommand)
{
    StreamWriterBuilder wBuilder;
    Value root;
    root["msg_id"] = m_msg_id++;
    root["command"] = nCommand;
    return QString(writeString(wBuilder, root).c_str());
}

QString AiPenLocalClient::buildAttendClass(int classId)
{
    StreamWriterBuilder wBuilder;
    Value root;
    root["msg_id"] = m_msg_id++;
    root["command"] = ePro_AttendClass;
    Value data;
    data["classId"] = classId;
    root["data"] = data;
    QString strAttendClassJson = QString(writeString(wBuilder, root).c_str());
    LOG(INFO) << "buildAttendClass: " << strAttendClassJson.toStdString();
    return strAttendClassJson;
//    return QString(writeString(wBuilder, root).c_str());
}

QString AiPenLocalClient::buildStartClasswork(const Json::Value &value)
{
    StreamWriterBuilder wBuilder;
    Value root;
    root["msg_id"] = m_msg_id++;
    root["command"] = ePro_StartClasswork;
    Value data = value;
    root["data"] = data;
    QString strWorkJson = QString(writeString(wBuilder, root).c_str());
    LOG(INFO) << "buildStartClasswork: " << strWorkJson.toStdString();
    return strWorkJson;
}

QString AiPenLocalClient::buildStartDictation(int workClassId)
{
    StreamWriterBuilder wBuilder;
    Value root;
    root["msg_id"] = m_msg_id++;
    root["command"] = ePro_StartDictation;
    Value data;
    data["workClassId"] = workClassId;
    root["data"] = data;
    QString strWorkJson = QString(writeString(wBuilder, root).c_str());
    LOG(INFO) << "buildStartDictation: " << strWorkJson.toStdString();
    return strWorkJson;
}

QString AiPenLocalClient::buildDictationWord(QString msg)
{
    string stdJsonStr = msg.toStdString();
    istringstream iss(stdJsonStr);
    CharReaderBuilder rBuilder;
    Value obj;
    String err;
    parseFromStream(rBuilder, iss, &obj, &err);

    StreamWriterBuilder wBuilder;
    Value root;
    root["msg_id"] = m_msg_id++;
    root["command"] = ePro_DictationWord;
    root["data"] = obj;
    QString strWorkJson = QString(writeString(wBuilder, root).c_str());
    LOG(INFO) << "buildStartDictation: " << strWorkJson.toStdString();
    return strWorkJson;
}

bool AiPenLocalClient::send(const QString &msg)
{
    LOG(INFO) << "AiPenLocalClient::send " << msg.toStdString();
    if(conection)
    {
        if(!m_bCon)
        {
            conection->connectToServer(AIPEN_SERVER_NAME);
        }
        if(conection->isOpen())
        {
            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_0);
            out << msg;
            m_mutex.lock();
            conection->write(block);
            m_mutex.unlock();
            return true;
        }
        else
        {
            LOG(INFO) << "AiPenLocalClient:send fail:" << msg.toStdString();
        }
    }
    else
    {
         LOG(INFO) << "AiPenLocalClient:send  not connect";
    }
    return false;
}
#ifdef SHOW_TEST_BTN
QString AiPenLocalClient::buildOpenOriginFile(QString filePathes)
{
    StreamWriterBuilder wBuilder;
    Value root;
    root["msg_id"] = m_msg_id++;
    root["command"] = ePro_OpenOriginFile;
    root["data"] = filePathes.toStdString();
    QString strWorkJson = QString(writeString(wBuilder, root).c_str());
    LOG(INFO) << "buildOpenOriginFile: " << strWorkJson.toStdString();
    return strWorkJson;
}

void AiPenLocalClient::openOriginFile(QString filePathes)
{
    send(buildOpenOriginFile(filePathes));
}
#endif
