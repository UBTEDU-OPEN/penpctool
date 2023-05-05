#ifndef WEBSOCKETSERVER_H
#define WEBSOCKETSERVER_H

#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QList>
#include <QMutex>
#include "messagehandler.h"
#include "ap.h"
#include "upgradeworker.h"

class WebSocketServer : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketServer(QObject *parent = nullptr);
    ~WebSocketServer();
    void onNewVersion();

    void sendNewVersionMsg();
    bool sendMsgToElectron(QString msg);

    void setUpgradeHandle(UpgradeWorker* workerHandle);

signals:
    void saveToken(QString token);
    void promptAboutDialog();
    void copyImgToClipboard(qulonglong);
    void notifyAttendClass(int classId);
    void notifyFinishClass();
    void notifyStartClasswork(QString strClasswork);
    void notifyStopClasswork();
    void notifyAccountManagePage(QString);
    void notifyStartDictation(int workClassId);
    void dictationWordStart(QString);
    void notifyStopDictation();
    void closeMainWindow();
    void startUpgrade();
    void cancelUpgrade();
    void upgradeFinished();
    void pickOriginFile();
    void pickJsonFolder();


public slots:
    void onSendTextMessage(QString msg);
    void sendNoDataResponse(QString msgId, QString cmd, int code, QString msg);
    void onDownloadProgress(int percent, QByteArray strSpeed, QByteArray strRemain);
    void onStartInstall();
    void onUpgradeResult(int code, QByteArray msg);
    void onPromptCloseWarning();
    
private slots:
    void onNewConnection();
    void onTextMessage(QString message);
    void clientDisconnected();

    //监控笔的断连
    void handleSSEConnectStatusData(PenInfo pen);
    void handlePenPowerChanged(PenInfo pen);

    //监控新笔的发现
    void handleSSEScanStatusData(QList<PenInfo> penList);

    //监控AP断连
    void handleAPStatusChangeData(APInfo apInfo);

private:
    QString buildConnectStatusResponse(QString id, int code, std::string msg, PenInfo& pen);
    QString buildPowerResponse(QString id, int code, std::string msg, PenInfo& pen);
    QString buildScanStatusResponse(QString id, int code, std::string msg, QList<PenInfo>& pens);
    QString buildAPStatusResponse(QString id, int code, std::string msg, APInfo& info);

    QString buildAboutInfo();
    QString buildUpgradeInfo();
    QString buildNewVersion();
    QString buildAttendClass();
    QString buildFinishClass();

private:
    QWebSocketServer* webSocketServer_ = nullptr;
    QWebSocket* webClient_ = nullptr;
    QWebSocket* electronClient_ = nullptr;
    MessageHandler* handler_ = nullptr;
    UpgradeWorker* upgradeHandle_ = nullptr;
};

#endif // WEBSOCKETSERVER_H
