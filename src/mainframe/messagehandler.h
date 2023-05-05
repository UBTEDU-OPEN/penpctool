#ifndef PENMESSAGEHANDLER_H
#define PENMESSAGEHANDLER_H

#include <QObject>
#include <QWebSocket>
#include <QRecursiveMutex>
#include <QMap>

#include "json/json.h"
#include "Classwork.h"

#define SaveToken  "saveToken"
#define AddAp  "addAp"
#define PenPair "penPair"
#define PenUnpair "penUnpair"
#define GetPenList "getPenList"
//笔的连接状态
#define RegisterConnectStatusEvent "registerAPConnectStatusEventListener"
#define UnregisterConnectStatusEvent "unregisterAPConnectStatusEventListener"
//扫描到哪些笔
#define RegisterScanStatusEvent "registerAPScanStatusEventListener"
#define UnregisterScanStatusEvent "unregisterAPScanStatusEventListener"
//路由器的连接状态
#define RegisterAPStatus "registerAPStatusListener"
#define UnregisterAPStatus "unregisterAPStatusListener"
#define NotifyAttendClass "notifyAttendClass"
#define NotifyFinishClass "notifyFinishClass"
#define NotifyStartClasswork "notifyStartClasswork"
#define NotifyStopClasswork "notifyStopClasswork"
#define NotifyStartDictation "notifyStartDictation"
#define NotifyStopDictation "notifyStopDictation"
#define NotifyAccountManagePage "notifyAccountManagePage"
#define FrontendLog "frontendLog"
#define SaveImgToClipbrd "saveImgToClipbrd"
#define WebSaveLoginInfo "webSaveLoginInfo"
#define WebLoggout "webLoggout"
#define WebGetApMacs "webGetApMacs"
#define WebSaveExerciseTimes "webSaveExerciseTimes"
#define WebGetExerciseTimes "webGetExerciseTimes"
#define RegisterUpdateState "registerUpdateStateListener"
#define UnregisterUpdateState "unregisterUpdateStateListener"
#define RegisterPowerEvent "registerPowerEventListener"
#define UnregisterPowerEvent "unregisterPowerEventListener"
#define DictationWordStart "dictationWordStart"
#define GetRecoveryInfo "getRecoveryInfo"

#define GetCommonConfig "getCommonConfig"
#define SaveCommonConfig "saveCommonConfig"

#define ElectronClient "electronClient"
#define PromptAboutDialog "promptAboutDialog"
#define PromptUpgradeDialog "promptUpgradeDialog"
#define StartUpgrade "startUpgrade"
#define DownloadInfo "downloadInfo"
#define StartInstall "startInstall"
#define UpgradeResult "upgradeResult"
#define UpgradeFinished "upgradeFinished"
#define CancelUpgrade "cancelUpgrade"
#define HasNewVersion "hasNewVersion"
#define PromptCloseWarning "promptCloseWarning"
#define OpenOfficalWebsite "openOfficalWebsite"
#define OpenLicenseWebsite "openLicenseWebsite"
#define OpenPrivacyWebsite "openPrivacyWebsite"
#define PickOriginFile "pickOriginFile"
#define PickJsonFolder "pickJsonFolder"

class MessageHandler : public QObject
{
    Q_OBJECT
public:
    explicit MessageHandler(QObject *parent = nullptr);
    ~MessageHandler();

    void onNewVersion();
    void handleMessage(QString message);

signals:
    void saveToken(QString token);

    void addAp(QString msgId, QList<QString> ipOrMacList);
    void penPair(QString msgId, QList<QString> macList);
    void penUnPair(QString msgId, QList<QString> macList);
    void getPenList(QString msgId, QList<QString> macList);

    void notifyAttendClass(int classId);
    void notifyFinishClass();
    void notifyStartClasswork(QString strClasswork);
    void notifyStopClasswork();
    void notifyAccountManagePage(QString);
    void notifyStartDictation(int);
    void notifyStopDictation();

    void sendTextMessage(QString msg);
    void promptAboutDialog();

    void copyImgToClipboard(qulonglong);

    void dictationWordStart(QString);
    void closeMainWindow();
    
public slots:

private:
    void handleSaveToken(const QString& msgId, const Json::Value& value);
    void handleAddAp(const QString& msgId, const Json::Value& value);
    void handlePenPair(const QString& msgId, const Json::Value& value);
    void handlePenUnPair(const QString& msgId, const Json::Value& value);
    void handleGetPenList(const QString& msgId, const Json::Value& value);
    void handleRegisterAPConnectStatusEventListener(const QString& msgId);
    void handleUnregisterAPConnectStatusEventListener(const QString& msgId);
    void handleRegisterAPScanStatusEventListener(const QString& msgId);
    void handleUnregisterAPScanStatusEventListener(const QString& msgId);
    void handleRegisterAPStatusListener(const QString& msgId);
    void handleUnregisterAPStatusListener(const QString& msgId);
    void handleNotifyAttendClass(const QString& msgId, const Json::Value& value);
    void handleNotifyFinishClass(const QString& msgId);
    void handleNotifyStartClasswork(const QString& msgId, const Json::Value& value);
    void handleNotifyStopClasswork(const QString& msgId);
    void handleNotifyStartDictation(const QString& msgId, const Json::Value& value);
    void handleNotifyStopDictation(const QString& msgId);
    void handleNotifyAccountManagePage(const QString& msgId,const Json::Value &value);
    void handleFrontendLog(const Json::Value &value);
    void handleSaveImgToClipbrd(const QString& msgId, const Json::Value &value);
    void handleWebSaveLoginInfo(const QString& msgId, const Json::Value &value);
    void handleWebLogout(const QString& msgId);
    void handleWebGetApMacs(const QString& msgId);
    void handleWebSaveExerciseTimes(const QString& msgId, const Json::Value &value);
    void handleWebGetExerciseTimes(const QString& msgId);
    void handlePromptAboutDialog(const QString& msgId);
    void handleDictationWordStart(const QString& msgId, const Json::Value &value);

    void handleRegisterUpdateStateListener(const QString& msgId);
    void handleUnregisterUpdateStateListener(const QString& msgId);
    void handleRegisterPowerEventListerner(const QString& msgId);
    void handleUnregisterPowerEventListerner(const QString& msgId);
    void handleGetRecoveryInfo(const QString& msgId);

    unsigned int counter;//上课时计时10分钟，如果没有收到心跳，则报错
    QString buildGetRecoveryResponse(QString id, int code, std::string msg);
    void handleGetCommonConfig(const QString& msgId);
    void handleSaveCommonConfig(const QString& msgId, const Json::Value &value);
    QString buildGetCommmonConfig(QString id);

public:
    static QString buildNoDataResponse(QString id, std::string cmd, int code, std::string msg);
    QMap<QString,QString> webMsgIds_;
    bool hasNewVersion_ = false;
};

#endif // PENMESSAGEHANDLER_H
