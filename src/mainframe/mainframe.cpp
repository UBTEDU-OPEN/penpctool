#include "mainframe.h"
#include "ui_mainframe.h"

#include <QHBoxLayout>
#include <vector>
//#include <QWebEngineSettings>
//#include <QWebEnginePage>
//#include <QWebEngineFullScreenRequest>
//#include <QWebEngineProfile>
#include <string>
#include <functional>
#include <QApplication>
#include <QDir>
#include <cstdlib>
#include <QMessageBox>
#include <QCloseEvent>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileDialog>
#include <QScreen>
#include <QMetaEnum>
#include <QEvent>

#include <QPushButton>
#include <QFileDialog>
#include <QTextStream>
#include <QFileInfo>
#include <QStandardPaths>
#include <QAction>
#include <QClipboard>
#include <QVariant>

#include "logHelper.h"
#include "httpconfig.h"
#include "platformport.h"
#include "ubtserver.h"
#include "TQLAPComm.h"
//#include "aipenmanager.h"
//#include "tqlsdkadapter.h"
#include "apmanager.h"

#include "testwidget.h"
#include "apmanager.h"
#include "getportuitls.h"
#include "aboutdialog.h"
#include "upgradeworker.h"
#include "upgradeprocessor.h"
#include "projectconst.h"

void MainFrame::initWsHandler()
{
    socketServer_->moveToThread(&webSocketWorkThread_);
    connect(socketServer_,&WebSocketServer::saveToken,UBTServer::instance(),&UBTServer::onSaveToken);
    connect(socketServer_,&WebSocketServer::saveToken,this,&MainFrame::onSaveToken);
    connect(socketServer_,&WebSocketServer::promptAboutDialog,this,&MainFrame::onAboutTriggered);
    connect(this,&MainFrame::newVersion,socketServer_,&WebSocketServer::onNewVersion);
    connect(this,&MainFrame::downloadProgress,socketServer_,&WebSocketServer::onDownloadProgress);
    connect(this,&MainFrame::startInstall,socketServer_,&WebSocketServer::onStartInstall);
    connect(this,&MainFrame::upgradeResult,socketServer_,&WebSocketServer::onUpgradeResult);
    connect(socketServer_,&WebSocketServer::copyImgToClipboard,this,&MainFrame::onCopyImgToClipboard);
    webSocketWorkThread_.start(); //TODO: socketServer_移动
    connect(socketServer_, &WebSocketServer::notifyAttendClass, m_pPenClient, &AiPenLocalClient::onNotifyAttendClass);
    connect(socketServer_, &WebSocketServer::notifyFinishClass, m_pPenClient, &AiPenLocalClient::onNotifyFinishClass);
    connect(socketServer_, &WebSocketServer::notifyStartClasswork, m_pPenClient, &AiPenLocalClient::onNotifyStartClasswork);
    connect(socketServer_, &WebSocketServer::notifyStopClasswork, m_pPenClient, &AiPenLocalClient::onNotifyStopClasswork);
    connect(socketServer_, &WebSocketServer::notifyStartDictation, m_pPenClient, &AiPenLocalClient::onNotifyStartDictation);
    connect(socketServer_, &WebSocketServer::notifyStopDictation, m_pPenClient, &AiPenLocalClient::onNotifyStopDictation);
    connect(socketServer_, &WebSocketServer::dictationWordStart, m_pPenClient, &AiPenLocalClient::onDictationWordStart);
    connect(socketServer_, &WebSocketServer::closeMainWindow, this, [this]{
        closeMainFrame();
    });
    connect(socketServer_, &WebSocketServer::pickOriginFile, this, &MainFrame::onOpenFile);
    connect(socketServer_, &WebSocketServer::pickJsonFolder, this, &MainFrame::onOpenJsonFolder);

}

void MainFrame::initUpgradeConnection()
{
    connect(socketServer_, &WebSocketServer::startUpgrade,
            upgradeWorker_, &UpgradeWorker::onStartUpgrade);
    connect(socketServer_, &WebSocketServer::upgradeFinished, this, [this]{
        onInstallFinished(newVersion_);
    });
    connect(socketServer_, &WebSocketServer::cancelUpgrade, upgradeWorker_,
            &UpgradeWorker::onCancelUpgrade);
    connect(upgradeWorker_,&UpgradeWorker::requestDownloadUpgrade,httpWorker_,&HttpWorker::onRequestDownloadUpgrade);
    connect(UBTServer::instance(),&UBTServer::upgradeDownloadProgress,upgradeWorker_,&UpgradeWorker::onDownloadProgress);
    connect(UBTServer::instance(),&UBTServer::upgradeDownloadFinished,upgradeWorker_,&UpgradeWorker::onDownloadFinished);
    connect(UBTServer::instance(),&UBTServer::upgradeDownloadError,upgradeWorker_,&UpgradeWorker::onDownloadError);

    connect(upgradeWorker_,&UpgradeWorker::installFinished,this,&MainFrame::onInstallFinished);
    connect(upgradeWorker_,&UpgradeWorker::forceQuit,this,&MainFrame::onForceQuit);
    connect(upgradeWorker_,&UpgradeWorker::upgradeCanceled,UBTServer::instance(),&UBTServer::stopUpgradeDownload);
    connect(upgradeWorker_,&UpgradeWorker::downloadProgress,this,&MainFrame::downloadProgress);
    connect(upgradeWorker_,&UpgradeWorker::upgradeResult,this,&MainFrame::upgradeResult);
    connect(upgradeWorker_,&UpgradeWorker::startInstall,this,&MainFrame::startInstall);
}

MainFrame::MainFrame(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainFrame)
    , firstCheckUpdate_(true)
    , socketServer_(new WebSocketServer)
    , m_pPenClient(new AiPenLocalClient)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose);
    //先初始化websocket
    initWsHandler();

    m_bIsCrash =  Config::GetAppCrashState();
    if(!m_bIsCrash) {
        if(Config::getAppRunning() && EvaluationCheckThread::checkAipenServerProc(false)
                && EvaluationCheckThread::checkProc(false)) {
            m_bIsCrash = true;
        }
    }

    if (!m_bIsCrash)
    {
        //清空自恢复保存的一些数据状态，如崩溃状态、上下课状态、心跳状态，因为是清空旧状态的初始化操作，所以放到启动开始的时候执行
        Config::SetHeartbeat(false);
        Config::saveClassInfo(false, 0);
        Config::saveClassWorkInfo(false, 0);
    }
    Config::SetAppCrashState(false);//清空是否崩溃状态
    Config::setAppRunning(true);

    int port = 0;
    if (m_bIsCrash)
    {
        Config::getPort(port);
    }
    else
    {
        GetPortUitls portUitls;
        QVector<uint16_t> ports =  portUitls.FindAvailablePort();
        port = ports.first(); //保存port
        Config::savePort(port);
    }

    checkThread.setMainProCrash(m_bIsCrash);
    checkThread.start();

    httpWorker_ = new HttpWorker;
    httpWorker_->moveToThread(&httpWorkThread_);

    UBTServer* ubtInst = UBTServer::instance();
    ubtInst->moveToThread(&httpWorkThread_);


    connect(this,&MainFrame::stopTimer,httpWorker_,&HttpWorker::onStopTimer);
    connect(ubtInst,&UBTServer::replyResult,httpWorker_,&HttpWorker::onReplyResult);

    APManager::getInstance()->port = port;
    LOG(INFO)<<"APManager::getInstance() port="<<port;
    APManager::getInstance()->scanAvailableAps(false);
    connect(APManager::getInstance(),&APManager::noFN,m_pPenClient,&AiPenLocalClient::onNoFN);

    qDebug() << "MainFrame thread id=" << QThread::currentThreadId();
    httpWorkThread_.start();

    connect(this,&MainFrame::httpRequest,httpWorker_,&HttpWorker::onHttpRequest);
    connect(UpgradeProcessor::instance(),&UpgradeProcessor::newVersion,this,&MainFrame::onNewVersion);
    connect(httpWorker_,&HttpWorker::sigHandleUpgradeInfo,UpgradeProcessor::instance(), &UpgradeProcessor::OnHandleUpgradeInfo);


    moduleUpdateWorker_ = new ModuleUpdateWorker;
    moduleUpdateWorker_->moveToThread(&moduleUpdateThread_);
    connect(UpgradeProcessor::instance(),&UpgradeProcessor::newCharEvaluation,moduleUpdateWorker_,&ModuleUpdateWorker::onNewCharEvaluation);
    connect(UpgradeProcessor::instance(),&UpgradeProcessor::newHandWritten,moduleUpdateWorker_,&ModuleUpdateWorker::onNewHandWritten);
    connect(UpgradeProcessor::instance(),&UpgradeProcessor::newAipenServer,moduleUpdateWorker_,&ModuleUpdateWorker::onNewAipenServer);
    connect(moduleUpdateWorker_,&ModuleUpdateWorker::requestDownloadPackage,httpWorker_,&HttpWorker::onRequestDownloadUpgrade);
    connect(UBTServer::instance(),&UBTServer::charEvaluationDownloadFinished,moduleUpdateWorker_,&ModuleUpdateWorker::onCharEvaluationDownloadFinished);
    connect(UBTServer::instance(),&UBTServer::handWrittenDownloadFinished,moduleUpdateWorker_,&ModuleUpdateWorker::onHandWrittenDownloadFinished);
    connect(UBTServer::instance(),&UBTServer::aipenServerDownloadFinished,moduleUpdateWorker_,&ModuleUpdateWorker::onAipenServerDownloadFinished);
    moduleUpdateThread_.start();


    std::string url = HttpConfig::instance()->getUpgradeUrl(MAIN_MODULE_NAME,
                                          PRODUCT_NAME,Config::localVersion().toStdString());
    emit httpRequest(QString::fromStdString(url),QByteArray(),1,MyRequestType::kRequestUpgradeUrl,0);


    QFile ceFile("charevaluation.json");
    if(ceFile.open(QFile::ReadOnly)) {
        auto doc = QJsonDocument::fromJson(ceFile.readAll());
        QJsonObject obj = doc.object();
        QString version = obj.value("version").toString();
        std::string url = HttpConfig::instance()->getUpgradeUrl(EVALUATION_MODULE_NAME,
                                                                PRODUCT_NAME,version.toStdString());
        emit httpRequest(QString::fromStdString(url),QByteArray(),1,MyRequestType::kRequestCharEvaluationUrl,0);
        ceFile.close();
    }
    QFile hwFile("handwritten.json");
    if(hwFile.open(QFile::ReadOnly)) {
        auto doc = QJsonDocument::fromJson(hwFile.readAll());
        QJsonObject obj = doc.object();
        QString version = obj.value("version").toString();
        std::string url = HttpConfig::instance()->getUpgradeUrl(HAND_WRITTEN_MODULE_NAME,
                                                                PRODUCT_NAME,version.toStdString());
        emit httpRequest(QString::fromStdString(url),QByteArray(),1,MyRequestType::kRequestHandWrittenUrl,0);
        hwFile.close();
    }
    QFile aipenFile("aipenserver.json");
    if(aipenFile.open(QFile::ReadOnly)) {
        auto doc = QJsonDocument::fromJson(aipenFile.readAll());
        QJsonObject obj = doc.object();
        QString version = obj.value("version").toString();
        std::string url = HttpConfig::instance()->getUpgradeUrl(SERVER_MODULE_NAME,
                                                                PRODUCT_NAME,version.toStdString());
        emit httpRequest(QString::fromStdString(url),QByteArray(),1,MyRequestType::kRequestAipenServerUrl,0);
        aipenFile.close();
    }
}

MainFrame::~MainFrame()
{
    LOG(INFO)<<"~MainFrame()" ;
    delete ui;
}

void MainFrame::pushWorkCommit(const QString& mac,int workClassId)
{
    std::string uploadUrl = HttpConfig::instance()->getWorkCommit();
    qDebug()<< "pushWorkCommit " << QString::fromStdString(uploadUrl) ;

    QJsonDocument jdoc;
    QJsonObject Obj;
    Obj["mac"] = mac;
    Obj["workClassId"] = workClassId;
    jdoc.setObject(Obj);
    QByteArray byte = jdoc.toJson(QJsonDocument::Indented);
    qDebug()<< "reportEvaluateResult " << QString::fromStdString(uploadUrl) ;
//    UBTServer::instance()->postData(QString::fromStdString(uploadUrl),byte,[=](int result,const QJsonObject& js1){
//        if(result == 0) {
//            qDebug()<< "pushWorkCommit success";
//        } else{
//              qDebug()<< "pushWorkCommit failed" << result;
//        }
//    });
}

void MainFrame::closeMainFrame()
{
    LOG(INFO) << "onClose called";
    Config::setAppRunning(false);
    moduleUpdateThread_.quit();
    moduleUpdateThread_.wait();

    webSocketWorkThread_.quit();
    webSocketWorkThread_.wait();

//    emit stopTimer();
//    disconnect(UBTServer::instance(),&UBTServer::replyResult,httpWorker_,&HttpWorker::onReplyResult);

//    disconnect(socketServer_);
    manager->deleteLater();

    delete APManager::getInstance();

    httpWorkThread_.quit();
    httpWorkThread_.wait();

    UBTServer::instance()->deleteLater();//这里不能直接释放，会出现异常，改为由自己释放，


//    socketServer_->deleteLater();

    checkThread.stopRunning();
    checkThread.wait();

    LOG(INFO) << "onClose end";
//    close();
    QGuiApplication::quit();
}

void MainFrame::handleMessage(const QString &)
{
    showMaximized(); //TODO: 需要保存electron的状态进行判断
}

void MainFrame::onSaveToken()
{
    static bool firstSaveToken = true;

    if(m_bIsCrash && firstSaveToken) {
        //crash后的首次登录不需要发送saveToken，避免自动结束当前的随堂练习
    } else {
        m_pPenClient->finishAllClassWork();
    }
    firstSaveToken = false;
}

void MainFrame::onAboutTriggered()
{
    std::string url = HttpConfig::instance()->getUpgradeUrl(MAIN_MODULE_NAME,
                                          PRODUCT_NAME,Config::localVersion().toStdString());
    emit httpRequest(QString::fromStdString(url),
                     QByteArray(),1,MyRequestType::kRequestUpgradeUrl,0);
}

void MainFrame::onNewVersion(bool isForced, QString packageMd5,
                             QString packageUrl, QString releaseNote,
                             QString versionName, qint64 packageSize)
{
    newVersion_ = versionName;
    if(nullptr != upgradeWorker_) {
        delete upgradeWorker_;
    }
    upgradeWorker_ = new UpgradeWorker(isForced,packageMd5,
                                       packageUrl,releaseNote,
                                       versionName,packageSize,
                                       firstCheckUpdate_,this);
    initUpgradeConnection();
    socketServer_->setUpgradeHandle(upgradeWorker_);
    if(firstCheckUpdate_) {
        firstCheckUpdate_ = false;
    }
    emit newVersion();
}

void MainFrame::onInstallFinished(QString newVersion)
{
    LOG(INFO) << "MainFrame::onInstallFinished";
    createNewLink(newVersion);
    closeMainFrame();
    QProcess::startDetached("../" + newVersion + "/aipen.exe",
                            QStringList(), "../" + newVersion);
}

void MainFrame::onForceQuit()
{
    closeMainFrame();
}

void MainFrame::createNewLink(QString newVersion)
{
    QString deskTopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    deskTopPath += QString::fromLocal8Bit("/小优AI练字.lnk");
    bool ret = QFile::exists(deskTopPath);
    LOG(INFO) << "deskTopPath exists " << ret;
    if(ret) {
        ret = QFile::remove(deskTopPath);
    }

    QString relativePath = QCoreApplication::applicationDirPath() + QString("/../%1").arg(newVersion);
    QString cleanPath = QDir::cleanPath(relativePath);
    cleanPath += "/aipen.exe";
    LOG(INFO) << "UpgradeProcessor::createNewLink " << cleanPath.toStdString();
    ret = QFile::link(cleanPath,deskTopPath);
}

void MainFrame::onCopyImgToClipboard(qulonglong pixmapPtr)
{
    QPixmap* image = (QPixmap*)pixmapPtr;
    QClipboard *clip=QApplication::clipboard();
    clip->setPixmap(*image);
    delete image;
}

#ifdef SHOW_TEST_BTN
void MainFrame::onOpenFile()
{
    auto fileList = QFileDialog::getOpenFileNames(this, tr("open origin data"), ".","(*.txt)");
    if(fileList.size() > 0) {
        m_pPenClient->openOriginFile(fileList.join(';'));
    }
}

void MainFrame::onOpenJsonFolder()
{
//    auto fileList = QFileDialog::getExistingDirectory(this, tr("open origin data"), ".","(*.txt)");
//    if(fileList.size() > 0) {
//        m_pPenClient->openOriginFile(fileList.join(';'));
//    }
}
#endif

