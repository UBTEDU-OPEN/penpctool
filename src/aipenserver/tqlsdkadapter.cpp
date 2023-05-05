#include "tqlsdkadapter.h"
#include "logHelper.h"
#include <QDebug>
#include "aipenmanager.h"
//#include "apmanager.h"

#include "config.h"

QMap<QString,int> TQLSDKAdapter::macToThreadId_;

bool TQLSDKAdapter::keepTempFile_ = false;
bool TQLSDKAdapter::showVerboseLog_ = false;

#ifdef SHOW_TEST_BTN
std::string TQLSDKAdapter::pMac = "AA:BB:CC:DD:EE:FF";
#endif

TQLSDKAdapter::TQLSDKAdapter(QObject *parent) : QObject(parent)
{
    keepTempFile_ = (1 == Config::getKeepTempFile());
    showVerboseLog_ = (1 == Config::getLogVerbose());
    LOG(INFO) <<"TQLSDKAdapter::TQLSDKAdapter: keepTempFile_=" << keepTempFile_;
    manager_ = new QNetworkAccessManager;


    if(keepTempFile_) {
        originDataSaver_ = new OriginDataSaver;
        originDataSaver_->moveToThread(&originDataSaveThread_);
        connect(this,&TQLSDKAdapter::originData,originDataSaver_,&OriginDataSaver::onOriginData);
        originDataSaveThread_.start();
    }

    penDotHandler1_ = new PenDotHandler(1);
    penDotHandler1_->moveToThread(&penDotHandleThread1_);
    connect(this,&TQLSDKAdapter::newDot1,penDotHandler1_,&PenDotHandler::onNewDot);
    penDotHandleThread1_.start();

    penDotHandler2_ = new PenDotHandler(2);
    penDotHandler2_->moveToThread(&penDotHandleThread2_);
    connect(this,&TQLSDKAdapter::newDot2,penDotHandler2_,&PenDotHandler::onNewDot);
    penDotHandleThread2_.start();

    penDotHandler3_ = new PenDotHandler(3);
    penDotHandler3_->moveToThread(&penDotHandleThread3_);
    connect(this,&TQLSDKAdapter::newDot3,penDotHandler3_,&PenDotHandler::onNewDot);
    penDotHandleThread3_.start();

}


TQLSDKAdapter::~TQLSDKAdapter()
{
    if(keepTempFile_) {
        originDataSaveThread_.quit();
        originDataSaveThread_.wait();
        originDataSaver_->releaseFile();
    }

    penDotHandleThread1_.quit();
    penDotHandleThread2_.quit();
    penDotHandleThread3_.quit();
    penDotHandleThread1_.wait();
    penDotHandleThread2_.wait();
    penDotHandleThread3_.wait();

    manager_->deleteLater();

    LOG(INFO) << "~TQLSDKAdapter";
}

TQLSDKAdapter* TQLSDKAdapter::getInstance()
{
    static TQLSDKAdapter* instance_ = new TQLSDKAdapter;
    return instance_;
}

int TQLSDKAdapter::init(int Port, int SocketType)
{
    int binit = Init(Port, SocketType);
    if(binit){
        //==========使用的回调===================
        InitCallBackReceiveDot(onReceivePenDot);
#ifdef SHOW_TEST_BTN
        InitCallBackImportReceiveDot(onReceiveImportPenDot);
#endif
        InitCallBackDotOuput(onReceivePenOriginDot);
        //InitCallBackElementCode(onPenElementCode);
        InitCallBackPower(onPenPower);
        InitFirmwareVersion(onPenFirmwareVersion);
        InitOffDataNumber(onPenOfflineDataSize);
        InitCallBackOfflineDataStatus(onPenOfflineDataStatus);
        //============未使用的回调===========
        //InitCallBackDebugger(onDebugger);
        InitCallBackHeartbeatPacket(onHeartbeatPacket);
        InitClientStatus(onClientStatus);
        //InitCallBackTcpDataPacket(onTcpDataPacket);
    }
    return binit;
}

//------------------------回调函数---------------------------------------
/**
 * @brief TQLSDKAdapter::onReceivePenDot -- 笔的点数据输出回调（包含：离线、实时）
 * @param mac
 * @param dot
 * @param Count
 */
void TQLSDKAdapter::onReceivePenDot(char *mac, Dot *dot, int Count,int channel)
{
    if(showVerboseLog_) {
        LOG(INFO) <<"TQLSDKAdapter::onReceivePenDot: mac="<<mac << ",type=" << dot->type
                << ",noteId=" << dot->noteId << ",pageId=" << dot->pageId
                << ",x=" <<dot->x << ",fx=" <<dot->fx <<",y="<<dot->y<<",fy=" << dot->fy
               << ",force=" << dot->force << ",time="<<dot->timeLong << ",count=" << Count << ",channel=" << channel;
    }

    QString macAddr(mac);
    if(!macToThreadId_.contains(macAddr)) {
        int size = macToThreadId_.size();
        int threadId = size % 3; //0,1,2
        macToThreadId_.insert(macAddr,threadId+1);
    }

    int threadId = macToThreadId_[macAddr];
    switch(threadId) {
    case 1:
        emit getInstance()->newDot1(QString(mac),dot->x,dot->fx,dot->y,dot->fy,dot->noteId,dot->pageId,(int)dot->type,dot->force,
                                    dot->timeLong);
        break;
    case 2:
        emit getInstance()->newDot2(QString(mac),dot->x,dot->fx,dot->y,dot->fy,dot->noteId,dot->pageId,(int)dot->type,dot->force,
                                    dot->timeLong);
        break;
    case 3:
        emit getInstance()->newDot3(QString(mac),dot->x,dot->fx,dot->y,dot->fy,dot->noteId,dot->pageId,(int)dot->type,dot->force,
                                    dot->timeLong);
        break;
    }
}

#ifdef SHOW_TEST_BTN
void  TQLSDKAdapter::onReceiveImportPenDot(Dot *dot, int Count,bool filter)
{
    qDebug() << "TQLSDKAdapter::onReceiveImportPenDot thread id=" << QThread::currentThreadId();
    LOG(INFO) <<"TQLSDKAdapter::onReceiveImportPenDot: mac="<< pMac << ",type=" << dot->type
            << ",noteId=" << dot->noteId << ",pageId=" << dot->pageId
            << ",x=" <<dot->x << ",fx=" <<dot->fx <<",y="<<dot->y<<",fy=" << dot->fy
           << ",force=" << dot->force << ",time="<<dot->timeLong<< ",count=" << Count << ",filter=" << filter;

    QString macAddr = QString::fromStdString(pMac);
    if(!macToThreadId_.contains(macAddr)) {
        int size = macToThreadId_.size();
        int threadId = size % 3; //0,1,2
        macToThreadId_.insert(macAddr,threadId+1);
    }

    int threadId = macToThreadId_[macAddr];
    switch(threadId) {
    case 1:
        emit getInstance()->newDot1(macAddr,dot->x,dot->fx,dot->y,dot->fy,dot->noteId,dot->pageId,(int)dot->type,dot->force,
                                    dot->timeLong);
        break;
    case 2:
        emit getInstance()->newDot2(macAddr,dot->x,dot->fx,dot->y,dot->fy,dot->noteId,dot->pageId,(int)dot->type,dot->force,
                                    dot->timeLong);
        break;
    case 3:
        emit getInstance()->newDot3(macAddr,dot->x,dot->fx,dot->y,dot->fy,dot->noteId,dot->pageId,(int)dot->type,dot->force,
                                    dot->timeLong);
        break;
    }
}
#endif

/**
 * @brief TQLSDKAdapter::onReceivePenOriginDot -- 笔原始数据输出回调
 * @param mac
 * @param pBuffer
 * @param Bufferlength
 */
void TQLSDKAdapter::onReceivePenOriginDot(char *mac,char *pBuffer, int Bufferlength)
{
    if(keepTempFile_) {
        qDebug() <<"TQLSDKAdapter::onReceivePenOriginDot: mac="<< mac << ",Bufferlength="<<Bufferlength;

        QString macAddr(mac);
        char* buf = new char[Bufferlength];
        memcpy_s(buf,Bufferlength,pBuffer,Bufferlength);
        emit getInstance()->originData(macAddr,buf,Bufferlength);
    }
}

/**
 * @brief TQLSDKAdapter::onPenElementCode -- 笔点读码数据输出回调
 * @param mac
 * @param SA
 * @param SB
 * @param SC
 * @param SD
 * @param Sindex
 */
void TQLSDKAdapter::onPenElementCode(char *mac, int SA, int SB, int SC, int SD, int Sindex)
{
    LOG(INFO)<<"TQLSDKAdapter::onPenElementCode: mac="<<mac<<", SA="<<SA<<",SB="<<SB<<",SC="<<SC<<",SD="<<SD<<",Sindex="<<Sindex;
}

/**
 * @brief TQLSDKAdapter::onPenPower -- 笔电量回调
 * @param mac
 * @param Battery
 * @param Charging
 */
void TQLSDKAdapter::onPenPower(char *mac, int Battery, int Charging)
{
    qDebug()<<"TQLSDKAdapter::onPenPower: mac="<<mac<<", Battery="<<Battery<<",Charging="<<Charging;
    emit getInstance()->sigPenPower(mac, Battery, Charging);
}
//如果>=6.7.2则开启慢书写模式，返回true；否则不开启，返回false
//ret 0表示>=6.7.2；1表示位数是V6.7这种格式，2表示版本号小于需求版本
int firmwareVersionMatched(char* FN)
{
    QString firmware = FN;
    QStringList versions = firmware.split('.');
    if(versions.size() < 3) {
        return 1;
    }

    if(versions[0].contains('V',Qt::CaseInsensitive)) {
        versions[0] = versions[0].mid(1);
    }
    qDebug() << "firmwareVersionMatched:" << versions;
    if(versions[0].toInt() > 6 ||
            (versions[0].toInt() == 6 && versions[1].toInt() > 7) ||
            (versions[0].toInt() == 6 && versions[1].toInt() == 7 && versions[1].toInt() >= 2)) {
        return 0;
    }

    return 2;
}

/**
 * @brief TQLSDKAdapter::onPenFirmwareVersion -- 笔版本信息回调
 * @param mac
 * @param FN
 */
void TQLSDKAdapter::onPenFirmwareVersion(char *mac, char *FN)
{
    LOG(INFO) << "TQLSDKAdapter::onPenFirmwareVersion: mac="<<mac<<", FN="<<FN;
    int ret = firmwareVersionMatched(FN);
    LOG(INFO) << "TQLSDKAdapter::onPenFirmwareVersion: mac="<<mac<<", ret="<<ret;
    if(!Config::testRestoreMode() && 0 == ret) {
        LOG(INFO) << "TQLSDKAdapter::onPenFirmwareVersion: mac="<<mac<<" set write speed 1.";
        ::setWriteSpeed(mac,1);
    }
    if(1 != ret) { //如果是V6.7只有两位的情况需要重新获取，不处理，处理0和2两种情况
        emit getInstance()->sigPenFirmwareVersion(mac, FN);
    }
}

/**
 * @brief TQLSDKAdapter::onPenOfflineDataSize -- 笔的离线数据量回调
 * @param mac
 * @param len
 */
void TQLSDKAdapter::onPenOfflineDataSize(char *mac, unsigned int len)
{
    LOG(INFO)<<"TQLSDKAdapter::onPenOfflineDataSize: mac="<<mac<<", len="<<len;
//    AiPenManager::getInstance()->handlePenOfflineDataSize(mac, len);
}

/**
 * @brief TQLSDKAdapter::onPenOfflineDataStatus -- 笔的离线数据获取状态的回调
 * @param mac
 * @param Status
 */
void TQLSDKAdapter::onPenOfflineDataStatus(char *mac,int Status)
{
    LOG(INFO)<<"TQLSDKAdapter::onPenOfflineDataStatus:mac="<<mac<<", status="<<Status;
//    AiPenManager::getInstance()->handlePenOfflineDataStatus(mac, Status);
}

//============================触发上述回调函数接口===========================
/**
 * @brief TQLSDKAdapter::doOpenReceiveDot -- 开始所有笔的点数据输出
 */
void TQLSDKAdapter::doOpenReceiveDot()
{
    LOG(INFO)<<"TQLSDKAdapter::doOpenReceiveDot";
//    OpenReceiveDot();
}

/**
 * @brief TQLSDKAdapter::doCloseReceiveDot -- 关闭所有笔的点数据输出
 */
void TQLSDKAdapter::doCloseReceiveDot()
{
    LOG(INFO)<<"TQLSDKAdapter::doCloseReceiveDot";
    CloseReceiveDot();
}

/**
 * @brief TQLSDKAdapter::doOpenReceivePenOfflineData -- 开始指定笔的离线点数据输出
 * @param ip
 * @param mac
 */
void TQLSDKAdapter::doOpenReceivePenOfflineData(char *ip,char *mac)
{
    LOG(INFO)<<"TQLSDKAdapter::doOpenReceivePenOfflineData:ip="<<ip<<", mac="<<mac;
    ApOfflineData(1, ip, mac);
}

/**
 * @brief TQLSDKAdapter::doCloseReceivePenOfflineData -- 关闭指定笔的离线点数据输出
 * @param ip
 * @param mac
 */
void TQLSDKAdapter::doCloseReceivePenOfflineData(char *ip,char *mac)
{
    LOG(INFO)<<"TQLSDKAdapter::doCloseReceivePenOfflineData:ip="<<ip<<", mac="<<mac;
    ApOfflineData(0, ip, mac);
}

/**
 * @brief TQLSDKAdapter::doCleanPenOfflineData -- 清空指定笔的离线点数据
 * @param ip
 * @param mac
 */
void TQLSDKAdapter::doCleanPenOfflineData(char *ip, char*mac)
{
    LOG(INFO)<<"TQLSDKAdapter::doCleanPenOfflineData:ip="<<ip<<", mac="<<mac;
    ApOfflineData(2, ip, mac);
}

/**
 * @brief TQLSDKAdapter::doGetPenOffDataNumber -- 获取指定笔的离线数据量
 * @param ip
 * @param mac
 * @return
 */
int TQLSDKAdapter::doGetPenOffDataNumber(char *ip, char *mac)
{
    LOG(INFO)<<"TQLSDKAdapter::doGetPenOffDataNumber:ip="<<ip<<", mac="<<mac;
    return GetPenOffDataNumber(ip, mac);
}

/**
 * @brief TQLSDKAdapter::doSetPenRTC -- 设置指定笔的RTC
 * @param ip
 * @param mac
 */
void TQLSDKAdapter::doSetPenRTC(char *ip, char *mac)
{
    LOG(INFO)<<"TQLSDKAdapter::SetPenRTC:ip="<<ip<<", mac="<<mac;
    SetPenRTC(ip, mac);
}

/**
 * @brief TQLSDKAdapter::doGetPenPower -- 获取指定笔的电量
 * @param ip
 * @param mac
 */
void TQLSDKAdapter::doGetPenPower(char *ip, char *mac)
{
    qDebug()<<"TQLSDKAdapter::doGetPenPower:ip="<<ip<<", mac="<<mac;
    std::string notify = "";
    std::string cmd = "";
    std::string Submac(mac);
    Submac = Submac.substr(0, 17);
    //通知
    notify.append("http://");
    notify.append(ip);
    notify.append("/gatt/nodes/");
    notify.append(Submac);
    notify.append("/handle/43/value/0100");
    //命令
    cmd.append("http://");
    cmd.append(ip);
    cmd.append("/gatt/nodes/");
    cmd.append(Submac);
    cmd.append("/handle/27/value/A801FF");
    QNetworkRequest request;
    request.setUrl(QUrl(notify.c_str()));
    QNetworkReply* reply = manager_->get(request);
    request.setUrl(QUrl(cmd.c_str()));
    reply = manager_->get(request);
}

/**
 * @brief TQLSDKAdapter::doGetPenFirmwareVersion -- 获取指定笔的版本信息
 * @param ip
 * @param mac
 */
void TQLSDKAdapter::doGetPenFirmwareVersion(char *ip, char *mac)
{
    LOG(INFO) <<"TQLSDKAdapter::GetPenFirmwareVersion:ip="<<ip<<", mac="<<mac;
    GetPenFirmwareVersion(ip, mac);
}

/**
 * @brief TQLSDKAdapter::onHeartbeatPacket -- 可能时ap与笔间的心跳
 * @param Packet
 * @param PacketLength
 */
void TQLSDKAdapter::onHeartbeatPacket(char * Packet, int PacketLength)
{
    LOG(INFO)<<"TQLSDKAdapter::onHeartbeatPacket:Packet="<<Packet<<",PacketLength="<<PacketLength;
    AiPenManager::getInstance()->setReceivedHeartbeatPacket();
}

/**
 * @brief TQLSDKAdapter::onClientStatus -- 可能是AP 状态？
 * @param ip
 * @param status
 */
void TQLSDKAdapter::onClientStatus(char *ip, int status)
{
    LOG(INFO)<<"TQLSDKAdapter::onClientStatus:ip="<<ip<<",status="<<status;
    //TODO: 用于获取AP状态，1是连接
}

/**
 * @brief TQLSDKAdapter::onDebugger -- 废弃的方法
 * @param pBuff
 * @param BuffLength
 */
void TQLSDKAdapter::onDebugger(char * pBuff, int BuffLength)
{
    qDebug()<<"TQLSDKAdapter::onDebugger:buf="<<QString::fromStdString(pBuff)<<",length:"<<BuffLength;
}

/**
 * @brief TQLSDKAdapter::onTcpDataPacket --- 不知道干嘛
 * @param pBuff
 * @param BuffLength
 */
void TQLSDKAdapter::onTcpDataPacket(char * pBuff, int BuffLength)
{
    qDebug()<<"TQLSDKAdapter::onTcpDataPacket:buf="<<pBuff<<",length:"<<BuffLength;
}

void TQLSDKAdapter::onOpenReceiveDot(unsigned char*lpData, char *mac,bool filter)
{
    Q_UNUSED(filter);
    LOG(INFO) << "TQLSDKAdapter::onOpenReceiveDot...";
    ::OpenReceiveDot(lpData,mac,true);
}
