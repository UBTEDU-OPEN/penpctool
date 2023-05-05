#ifndef APMANAGER_H
#define APMANAGER_H

#include <QObject>
#include "ap.h"
//#include "searchipformac.h"
#include "logHelper.h"
#include "config.h"
#include <QNetworkAccessManager>
#include <QRecursiveMutex>
#include <QTimer>
#include <QMap>

/**
 * @brief The SearchIpForMac class
 * 根据ap的Mac地址搜索ap的ip地址
 */
typedef struct MacAndIp
{
    QString mac;
    QString ip;

}MacAndIp;

class APManager : public QObject
{
    Q_OBJECT

private:
    explicit APManager(QObject *parent = nullptr);
    static APManager* instance_;

public:
    static APManager* getInstance();
    ~APManager();
    int port;
    void handlePenPower(QString mac, int Battery, int Charging);
    void handlePenFirmwareVersion(QString mac, QString FN);
    void saveMacIp(QString mac, QString ip);
    void removeMac(QString mac);
    QString getIpOfMac(QString mac);
    static QStringList getLocalIps();


signals:
    void sendNoDataResponse(QString msgId, QString cmd, int code, QString msg);
    void sendBuiltMsg(QString msg);
    void SSEConnectStatusData(PenInfo info);
    void PenPowerChanged(PenInfo info);
    void SSEScanStatusData(QList<PenInfo> penInfoList);
    void ApStatusData(APInfo info);
    void noFN(QString mac, QString ip);

public slots:
    void onAddAp(QString msgId, QList<QString> ipOrMacList);
    void onPenPair(QString msgId, QList<QString> macList);
    void onPenUnPair(QString msgId, QList<QString> macList);
    void onGetPenList(QString msgId, QList<QString> macList);

    //监控AP断连
    void handleAPStatusChangeData(APInfo apInfo);
    void onScanApTimeout();

public:
    APInfo getAPInfo(const QString& apid);
    QList<APInfo> getApList();
    ServerInfo getServerInfo(const QString& apid);
    void startScan();
    void stopScan();

    void scanAvailableAps(bool emitMsg, QString msgId = "");

private:
    QList<Ap*> apList_;
    QMap<QString,QString> macIpMap_;
    QNetworkAccessManager manager_;
    QRecursiveMutex macIpMutex_;
    QSet<QString> pendingMacs_;
    QRecursiveMutex pendingMacsMutex_;
    QTimer* apScanTimer_ = nullptr;
    QString tempAddApMsgId_;

private:
    QString buildGetPenListResponse(QString id, int code, std::string msg, QList<PenInfo>& pens);
    QString buildAddApResponse(QString id, int code, std::string msg, QString mac);
    bool ApHasSetuped(QString ip);
    bool isApIpValid(QString mac, QString ip);
    QMap<QString,QString> getMacIp();
    void setupAp(MacAndIp macIp, QString msgId, QString localAddr);
};

#endif // APMANAGER_HO
