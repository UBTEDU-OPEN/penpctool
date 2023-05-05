#ifndef MAINFRAME_H
#define MAINFRAME_H

#include <QMainWindow>
//#include <QWebEngineView>
#include <string>
#include <memory>
#include <vector>
#include "websocketserver.h"
//#include"searchipformac.h"
//#include "aipen.h"

#include <QNetworkAccessManager>
#include<QNetworkReply>
#include<QNetworkRequest>
#include<QUrl>
#include<QJsonObject>
#include <QProcess>
#include <QMenu>

#include "testdialog.h"
#include "fullscreenwindow.h"
#include "httpworker.h"
#include "evaluationcheckthread.h"
#include "customstyledmenu.h"
#include "upgradeworker.h"
#include "aboutdialog.h"
#include "promptdialog.h"
#include "moduleupdateworker.h"
#include "aipenclient.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainFrame; }
QT_END_NAMESPACE

class MainFrame : public QMainWindow
{
    Q_OBJECT

public:
    MainFrame(QWidget *parent = nullptr);
    ~MainFrame();

    /*上传用户作业完成*/
    void pushWorkCommit(const QString& mac,int workClassId);
    void closeMainFrame();


public slots:
    void handleMessage(const QString &);
    void onSaveToken();
    void onAboutTriggered();
    void onNewVersion(bool isForced, QString packageMd5, QString packageUrl,
                      QString releaseNote, QString versionName, qint64 packageSize);
    void onInstallFinished(QString newVersion);
    void onForceQuit();
    void createNewLink(QString newVersion);
    void onCopyImgToClipboard(qulonglong pixmapPtr);
#ifdef SHOW_TEST_BTN
    void onOpenFile();
    void onOpenJsonFolder();
#endif

signals:
    void stopTimer();
    void httpRequest(QString,QByteArray,int,int,qulonglong);
    void newVersion();
    void downloadProgress(int percent, QByteArray strSpeed, QByteArray strRemain);
    void startInstall();
    void upgradeResult(int code, QByteArray msg);

private:
    void initWsHandler();
    void initUpgradeConnection();

private:
    Ui::MainFrame *ui;
    WebSocketServer *socketServer_ = nullptr;
    QNetworkAccessManager *manager = nullptr;
    QThread httpWorkThread_;
    HttpWorker* httpWorker_ = nullptr;
    EvaluationCheckThread checkThread;
    QThread webSocketWorkThread_;
    bool firstCheckUpdate_ = true;
    QThread moduleUpdateThread_;
    ModuleUpdateWorker* moduleUpdateWorker_ = nullptr;
    AiPenLocalClient* m_pPenClient = nullptr;
    bool m_bIsCrash = false;//是否崩溃重启
    UpgradeWorker* upgradeWorker_ = nullptr;
    QString newVersion_;
};
#endif // MAINFRAME_H
