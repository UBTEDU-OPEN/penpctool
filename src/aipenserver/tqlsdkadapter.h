#ifndef TQLSDKADAPTER_H
#define TQLSDKADAPTER_H

#include <QObject>
#include "TQLAPComm.h"
#include <QNetworkAccessManager>
#include <QFile>
#include <QMap>
#include <QThread>

#include "origindatasaver.h"
#include "pendothandler.h"

class  TQLSDKAdapter : public QObject
{
    Q_OBJECT
public:
    ~TQLSDKAdapter();
private:
    explicit TQLSDKAdapter(QObject *parent = nullptr);
     QNetworkAccessManager *manager_;

public:
    int init(int Port, int SocketType);
    static TQLSDKAdapter* getInstance();
#ifdef SHOW_TEST_BTN
    static std::string pMac;
#endif

signals:
    void originData(QString macAddr, char* data, int len);
    void newDot1(QString mac, int x, int fx, int y, int fy, int noteId, int pageId, int type, int force, long long timeStamp);
    void newDot2(QString mac, int x, int fx, int y, int fy, int noteId, int pageId, int type, int force, long long timeStamp);
    void newDot3(QString mac, int x, int fx, int y, int fy, int noteId, int pageId, int type, int force, long long timeStamp);
    void sigPenPower(QString mac, int Battery, int Charging);
    void sigPenFirmwareVersion(QString mac, QString FN);

private:
    //------------------------回调函数---------------------------------------
    //笔的点数据输出回调（包含：离线、实时）
    static void  onReceivePenDot(char *mac, Dot *dot, int Count,int channel);
#ifdef SHOW_TEST_BTN
    //获取还原数据
    static void  onReceiveImportPenDot(Dot *dot, int Count,bool filter);
#endif
    //笔原始数据输出回调
    static void  onReceivePenOriginDot(char *mac,char *pBuffer, int Bufferlength);
    //笔点读码数据输出回调
    static void  onPenElementCode(char *mac, int SA, int SB, int SC, int SD, int Sindex);
    //笔电量回调
    static void  onPenPower(char *mac, int Battery, int Charging);
    //笔版本信息回调
    static void  onPenFirmwareVersion(char *mac, char *FN);
    //笔的离线数据量回调
    static void  onPenOfflineDataSize(char *mac, unsigned int len);
    //笔的离线数据获取状态的回调
    //Status=0 离线数据结束(关闭)
    //Status=1 开始离线数据 (获取)
    //Status=2 离线数据正在上传
    static void  onPenOfflineDataStatus(char *mac,int Status);

public:
    //============================触发上述回调函数接口===========================
    /**
     * @brief doOpenReceiveDot -- 开始所有笔的点数据输出
     */
    void doOpenReceiveDot();
    /**
     * @brief doCloseReceiveDot -- 关闭所有笔的点数据输出
     */
    void doCloseReceiveDot();

    /**
     * @brief doOpenReceivePenOfflineData -- 开始指定笔的离线点数据输出
     * @param ip
     * @param mac
     */
    void doOpenReceivePenOfflineData(char *ip,char *mac);
    /**
     * @brief doCloseReceivePenOfflineData -- 关闭指定笔的离线点数据输出
     * @param ip
     * @param mac
     */
    void doCloseReceivePenOfflineData(char *ip,char *mac);
    /**
     * @brief doCleanPenOfflineData -- 清空指定笔的离线点数据
     * @param ip
     * @param mac
     */
    void doCleanPenOfflineData(char *ip, char* mac);
    /**
     * @brief doGetPenOffDataNumber -- 获取指定笔的离线数据量
     * @param ip
     * @param mac
     * @return
     */
    int doGetPenOffDataNumber(char *ip, char *mac);

    /**
     * @brief doSetPenRTC -- 设置指定笔的RTC
     * @param ip
     * @param mac
     */
    void doSetPenRTC(char *ip, char *mac);
    /**
     * @brief doGetPenPower -- 获取指定笔的电量
     * @param ip
     * @param mac
     */
    void doGetPenPower(char *ip, char *mac);
    /**
     * @brief doGetPenFirmwareVersion -- 获取指定笔的版本信息
     * @param ip
     * @param mac
     */
    void doGetPenFirmwareVersion(char *ip, char *mac);
    void onOpenReceiveDot(unsigned char*lpData, char *mac,bool filter);

private:
    //心跳？
    void static onHeartbeatPacket(char * Packet, int PacketLength);
    //AP状态？
    void static onClientStatus(char *ip, int status);//??
    void static onDebugger(char * pBuff, int BuffLength);//??
    void static onTcpDataPacket(char * pBuff, int BuffLength);//??
    QThread originDataSaveThread_;
    OriginDataSaver* originDataSaver_;
    QThread penDotHandleThread1_;
    QThread penDotHandleThread2_;
    QThread penDotHandleThread3_;
    PenDotHandler* penDotHandler1_;
    PenDotHandler* penDotHandler2_;
    PenDotHandler* penDotHandler3_;
    static QMap<QString,int> macToThreadId_;

    static bool keepTempFile_;
    static bool showVerboseLog_;
};

#endif // TQLSDKADAPTER_H
