#ifndef AP_H
#define AP_H

#include <QObject>
#include "json/json.h"
#include <QNetworkAccessManager>
#include "QNetworkConfigurationManager"
#include <QNetworkReply>
#include <QTimer>
#include <atomic>

#define ScanStatusEvent "scanSseStatus"
#define ScanDataEvent "scanSseData"
#define ConnectionStatusEvent "connectStatusSseData"
#define SCANNING "scanning"
#define CFGCHANGED "cfgchanged"
#define DISCONNECTED "disconnected"
#define RECONNECTING "reconnecting"

typedef struct PenInfo {
    QString mac;
    QString name;
    QString flag;
    QString FN;//固件版本，目前未使用
    qlonglong timestamp = 0ll;
    int connected = 0;
    int battery = 0;
    int charging = 0;
    int paired = 0;//辅助字段，记录笔在该AP下的配对信息
}PenInfo;

typedef struct ServerInfo {
    QString ip;
    QString proto;
    QString type;
    int port;
}ServerInfo;

typedef struct  APInfo {
    QString ip;
    QString netmask;
    QString mac;
    QString fwVersion;
    QString appVersion;
    QString status;
}APInfo;


class JsonResponse
{
public:
    int code;
    QString msg;
    Json::Value data;
};

class Ap : public QObject
{
    Q_OBJECT
public:
    explicit Ap(QString ip, QString mac, QObject *parent = nullptr);
    ~Ap();

signals:
    void SSEConnectStatusData(PenInfo info);
    void PenPowerChanged(PenInfo info);
    void SSEScanStatusData(QList<PenInfo> penInfoList);
    void APStatusChangeData(APInfo apInfo);
    void noFN(QString mac, QString ip);

public:
    bool isSetuped();

    APInfo getAPInfo();

    ServerInfo getServerInfo();

    //通过TQLSDK， 获取笔真实电量
    int updatePenPower(QString mac, int Battery, int Charging);
    //通过TQLSDK, 获取笔固件版本
    int updatePenFirmwareVersion(QString mac, QString FN);

    //设置配置信息
    void setup(const ServerInfo& info, std::function<void(int code, QString msg)> callback);
    //启动ap扫描
    void startScan(std::function<void(int code, QString msg)> callback);

    //停止ap扫描
    void stopScan(std::function<void(int code, QString msg)> callback);

    //获取扫描状态
    void getScanStatus(std::function<void (int code,QString msg)> callback);

    //获取完整的已绑定设备列表
    void getPairedPens(std::function<void(int code, QString msg, QList<PenInfo> pens)> callback);

    //获取扫描缓存列表
    void getUnpairedPens(std::function<void(int code, QString msg, QList<PenInfo> pens)> callback);

    QList<PenInfo> getAllPens();
    QList<QString> getCurrentPairedPenMacList();

    //根据Mac地址绑定对应的笔
    void pairPens(QList<QString> macList, std::function<void(int code, QString msg)> callback);

    //根据Mac地址解绑对应的笔
    void unpairPens(QList<QString> macList, std::function<void(int code, QString msg)> callback);

    //根据Mac地址清除指定设备扫描缓存列表
    void clearUnpairedPens(QList<QString> macList, std::function<void(int code, QString msg)> callback);

    //清除所有设备扫描缓存列表
    void clearAllUnpairPens(std::function<void(int code, QString msg)> callback);


    void reboot(std::function<void(int code, QString msg)> callback);

private:
    void setServerInfo(const ServerInfo& info, std::function<void(int code, QString msg)> callback);
    void getServerInfo(std::function<void(int code, QString msg, ServerInfo* response)> callback);

    void getApInfo(std::function<void(int code, QString msg, APInfo* response)> callback);

    //前端通知SSE接口
    void startEventObserver();
    void stopEventObserver();
    void SSENotify();
    bool sseStreamParse(QString jsonLine);
    void sseCheckApServerinfo();
    void restartSseStream();

private slots:
    void sseStreamReceived();
    void sseStreamFinished();
    void sseStreamError(QNetworkReply::NetworkError error);
    void sseStreamTimer();
    void sseReconnectTimeout();

private:
    void doGet(QString api, std::function<void(JsonResponse response)> callback);
    void doPost(QString api, QString params, std::function<void(JsonResponse response)> callback);

public:
    const QString ip_;
    const QString mac_;

private:
    APInfo* apInfo_;
    ServerInfo* serverInfo_;
    QMap<QString, PenInfo*> pairedPenList_;
    QMap<QString, PenInfo*> scanedPenList_;
    bool setuped;

    QNetworkAccessManager *manager_;

    QTimer* setupTimer_;
    QAtomicInt isGetApInfoing_;
    bool startNotify;
    QAtomicInt needStartSseTimer_;
    qint64 timestamp_;
    QNetworkReply* m_reply;
    int m_retries = 0;
    QStringList sseStreamBufferList_;
    QStringList m_penConnectNotifyList;//笔的连接状态通知列表，用于恢复重启后，笔的状态更新到web
    QTimer* sseReconnectTimer_ = nullptr;

    QAtomicInt sseReconnecting_;
};

#endif // AP_H
