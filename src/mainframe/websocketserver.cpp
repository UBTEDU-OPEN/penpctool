#include "websocketserver.h"
#include <QMetaType>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QSslConfiguration>
#include <QDesktopServices>
#include "apmanager.h"
//#include "aipenmanager.h"
#include <logHelper.h>
#include <QPixmap>
#include <QApplication>
#include <QClipboard>
#include <QUuid>

#include "json/json.h"
#include "projectconst.h"
#include "moduleinfo.h"

using namespace Json;

WebSocketServer::WebSocketServer(QObject *parent) : QObject(parent)
{
    handler_ = new MessageHandler();
    webSocketServer_ = new QWebSocketServer(QStringLiteral("AIPenServer"),
                                            QWebSocketServer::NonSecureMode, this);
    //ssl socket 可参考https://blog.csdn.net/cqchengdan/article/details/97619483
    if(webSocketServer_->listen(QHostAddress::Any, 1234)) {
        connect(webSocketServer_, &QWebSocketServer::newConnection,
                this, &WebSocketServer::onNewConnection);

        connect(handler_, &MessageHandler::addAp, APManager::getInstance(), &APManager::onAddAp);
        connect(handler_, &MessageHandler::penPair, APManager::getInstance(), &APManager::onPenPair);
        connect(handler_, &MessageHandler::penUnPair, APManager::getInstance(), &APManager::onPenUnPair);
        connect(handler_, &MessageHandler::getPenList, APManager::getInstance(), &APManager::onGetPenList);

        connect(handler_, &MessageHandler::notifyAttendClass, this, &WebSocketServer::notifyAttendClass);
        connect(handler_, &MessageHandler::notifyFinishClass, this, &WebSocketServer::notifyFinishClass);
        connect(handler_, &MessageHandler::notifyStartClasswork, this, &WebSocketServer::notifyStartClasswork);
        connect(handler_, &MessageHandler::notifyStopClasswork, this, &WebSocketServer::notifyStopClasswork);
        connect(handler_, &MessageHandler::notifyStartDictation, this, &WebSocketServer::notifyStartDictation);
        connect(handler_, &MessageHandler::notifyStopDictation, this, &WebSocketServer::notifyStopDictation);
        connect(handler_, &MessageHandler::notifyAccountManagePage,[](QString url){
            QDesktopServices::openUrl(url);
        });
        connect(handler_,&MessageHandler::dictationWordStart,this, &WebSocketServer::dictationWordStart);
        connect(handler_,&MessageHandler::closeMainWindow,this, &WebSocketServer::closeMainWindow);

        //统一将回socket消息由server处理，因为外部websocket*引用的对象可能已被server释放
        connect(handler_, &MessageHandler::sendTextMessage, this, &WebSocketServer::onSendTextMessage);
        connect(APManager::getInstance(), &APManager::sendBuiltMsg, this, &WebSocketServer::onSendTextMessage);
        connect(APManager::getInstance(), &APManager::sendNoDataResponse, this, &WebSocketServer::sendNoDataResponse);
        connect(APManager::getInstance(), &APManager::SSEConnectStatusData, this, &WebSocketServer::handleSSEConnectStatusData);
        connect(APManager::getInstance(), &APManager::PenPowerChanged, this, &WebSocketServer::handlePenPowerChanged);
        connect(APManager::getInstance(), &APManager::SSEScanStatusData, this, &WebSocketServer::handleSSEScanStatusData);
        connect(APManager::getInstance(), &APManager::ApStatusData, this, &WebSocketServer::handleAPStatusChangeData);
        connect(handler_,&MessageHandler::saveToken,this,&WebSocketServer::saveToken);
        connect(handler_,&MessageHandler::promptAboutDialog,this,&WebSocketServer::promptAboutDialog);
        connect(handler_,&MessageHandler::copyImgToClipboard,this,&WebSocketServer::copyImgToClipboard);
    } else {
        LOG(WARNING) << "WebSocketServer failed to lisen";
    }

}

WebSocketServer::~WebSocketServer()
{
    qDebug()<<"~WebSocketServer()";
    webSocketServer_->close();
    delete handler_;
}

void WebSocketServer::sendNewVersionMsg()
{
    if(handler_->webMsgIds_.contains(QString(RegisterUpdateState))) {
        StreamWriterBuilder wBuilder;
        Value root;
        root["id"] = handler_->webMsgIds_[QString(RegisterUpdateState)].toStdString();
        root["command"] = RegisterUpdateState;
        Value body;
        body["code"] = 0;
        body["msg"] = "success";
        Value data;
        data["needUpdate"] = true;
        body["data"] = data;
        root["body"] = body;
        QString msg = QString(writeString(wBuilder, root).c_str());
        onSendTextMessage(msg);
    }
}

bool WebSocketServer::sendMsgToElectron(QString msg)
{
    if(nullptr != electronClient_ && !msg.isEmpty()) {
        electronClient_->sendTextMessage(msg);
        return true;
    }
    return false;
}

void WebSocketServer::setUpgradeHandle(UpgradeWorker *workerHandle)
{
    upgradeHandle_ = workerHandle;
}

void WebSocketServer::onNewVersion()
{
    sendNewVersionMsg();
    if(nullptr != upgradeHandle_) {
        if(upgradeHandle_->firstCheck()) {
            if(nullptr != electronClient_) {
                sendMsgToElectron(buildUpgradeInfo());
                upgradeHandle_->resetFirstCheck();
            }
        } else {
            sendMsgToElectron(buildNewVersion());
        }
    }
}

//1对1
void WebSocketServer::onNewConnection()
{
//    qDebug() << "threadid onNewConnection" << QThread::currentThreadId();
    LOG(INFO) << "WebSocketServer::onNewConnection";
    QWebSocket *socketClient = webSocketServer_->nextPendingConnection();
    if(nullptr != socketClient) {
        connect(socketClient, &QWebSocket::textMessageReceived, this, &WebSocketServer::onTextMessage);
        connect(socketClient, &QWebSocket::binaryMessageReceived, this, [](const QByteArray &message){
            LOG(INFO) << "binaryMessageReceived:" << message.data();
        });
        connect(socketClient, &QWebSocket::disconnected, this, &WebSocketServer::clientDisconnected);
    }
}

void WebSocketServer::clientDisconnected()
{
    QWebSocket *socketClient = qobject_cast<QWebSocket *>(sender());
    LOG(INFO) << "socketDisconnected:" << socketClient;
    if(socketClient == electronClient_) {
        electronClient_ = nullptr;
    } else if(socketClient == webClient_) {
        webClient_ = nullptr;
    }
    socketClient->deleteLater();
}

void WebSocketServer::onTextMessage(QString message)
{
    LOG(INFO) << "Message received:" << message.toStdString();
    QWebSocket *socketClient = qobject_cast<QWebSocket *>(sender());
    QJsonObject obj = QJsonDocument::fromJson(message.toUtf8()).object();
    LOG(INFO) << "WebSocketServer::onTextMessage ID=" << obj["id"].toString().toStdString();
    QString command = obj["command"].toString();
    if(command == PromptAboutDialog) {
        sendMsgToElectron(buildAboutInfo());
        emit promptAboutDialog();
    } else if(command == ElectronClient) {
        electronClient_ = socketClient;
        LOG(INFO) << "ElectronClient upgradeHandle_=" << upgradeHandle_;
        if(upgradeHandle_ && upgradeHandle_->firstCheck()) {
            LOG(INFO) << "ElectronClient firstCheck";
            upgradeHandle_->resetFirstCheck();
            auto ret = electronClient_->sendTextMessage(buildUpgradeInfo());
            LOG(INFO) << "ElectronClient firstCheck send ret=" << ret;
        }
    } else if(command == RegisterAPStatus) {
        LOG(INFO) << "RegisterAPStatus upgradeHandle_=" << upgradeHandle_;
        webClient_ = socketClient;
        handler_->handleMessage(message);
    } else if(command == RegisterUpdateState) {
        handler_->handleMessage(message);
        if(nullptr != upgradeHandle_) {
            sendNewVersionMsg();
        }
    } else if(command == NotifyAttendClass) {
        sendMsgToElectron(buildAttendClass());
        handler_->handleMessage(message);
    } else if(command == NotifyFinishClass) {
        sendMsgToElectron(buildFinishClass());
        handler_->handleMessage(message);
    } else if(command == StartUpgrade) {
        emit startUpgrade();
    } else if(command == CancelUpgrade) {
        emit cancelUpgrade();
    } else if(command == UpgradeFinished) {
        emit upgradeFinished();
    } else if(command == OpenOfficalWebsite) {
//        QDesktopServices::openUrl(QUrl(QString::fromStdString(OFFICIAL_URL)));
    } else if(command == OpenLicenseWebsite) {
//        QDesktopServices::openUrl(QUrl(QString::fromStdString(LICENSE_URL)));
    } else if(command == OpenPrivacyWebsite) {
//        QDesktopServices::openUrl(QUrl(QString::fromStdString(PRIVACY_URL)));
    } else if(command == PickOriginFile) {
        emit pickOriginFile();
    } else if(command == PickJsonFolder) {
        emit pickJsonFolder();
    } else {
        handler_->handleMessage(message);
    }
}

void WebSocketServer::onSendTextMessage(QString msg)
{
    qDebug() << "onSendTextMessage" << msg;
    if(nullptr != webClient_) {
        webClient_->sendTextMessage(msg);
    }
}

void WebSocketServer::sendNoDataResponse(QString msgId, QString cmd, int code, QString msg)
{
    QString jsonMsg = MessageHandler::buildNoDataResponse(msgId,cmd.toStdString(),code,msg.toStdString());
    onSendTextMessage(jsonMsg);
}

void WebSocketServer::onDownloadProgress(int percent, QByteArray strSpeed, QByteArray strRemain)
{
    StreamWriterBuilder wBuilder;
    Value root;
    root["id"] = QUuid::createUuid().toString().remove("{").remove("}").toStdString();
    root["command"] = DownloadInfo;
    Value body;
    Value data;
    data["percent"] = percent;
    data["speed"] = strSpeed.toStdString();
    data["remain"] = strRemain.toStdString();

    body["data"] = data;
    root["body"] = body;
    sendMsgToElectron(QString(writeString(wBuilder, root).c_str()));
}

void WebSocketServer::onStartInstall()
{
    StreamWriterBuilder wBuilder;
    Value root;
    root["id"] = QUuid::createUuid().toString().remove("{").remove("}").toStdString();
    root["command"] = StartInstall;
    sendMsgToElectron(QString(writeString(wBuilder, root).c_str()));
}

void WebSocketServer::onUpgradeResult(int code, QByteArray msg)
{
    StreamWriterBuilder wBuilder;
    Value root;
    root["id"] = QUuid::createUuid().toString().remove("{").remove("}").toStdString();
    root["command"] = UpgradeResult;
    Value body;
    body["code"] = code;
    body["msg"] = msg.toStdString();
    root["body"] = body;

    sendMsgToElectron(QString(writeString(wBuilder, root).c_str()));
}

void WebSocketServer::onPromptCloseWarning()
{
    StreamWriterBuilder wBuilder;
    Value root;
    root["id"] = QUuid::createUuid().toString().remove("{").remove("}").toStdString();
    root["command"] = PromptCloseWarning;
    sendMsgToElectron(QString(writeString(wBuilder, root).c_str()));
}

void WebSocketServer::handleSSEConnectStatusData(PenInfo pen)
{
    LOG(INFO)<<"connectStatus: mac="<<pen.mac.toStdString()<<",connected="<<pen.connected<<", paired="<<pen.paired;

    if(handler_->webMsgIds_.contains(QString(RegisterConnectStatusEvent))) {
        onSendTextMessage(buildConnectStatusResponse(handler_->webMsgIds_[QString(RegisterConnectStatusEvent)], 0, "success", pen));
    }
}

void WebSocketServer::handlePenPowerChanged(PenInfo pen)
{
    LOG(INFO)<<"handlePenPowerChanged: mac="<<pen.mac.toStdString()<<",battery="<<pen.battery<<", charging="<<pen.charging;

    if(handler_->webMsgIds_.contains(QString(RegisterPowerEvent))) {
        onSendTextMessage(buildPowerResponse(handler_->webMsgIds_[QString(RegisterPowerEvent)], 0, "success", pen));
    }
}

void WebSocketServer::handleSSEScanStatusData(QList<PenInfo> penList)
{
    for(PenInfo&p: penList){
        LOG(INFO)<<"scanStatus: mac="<<p.mac.toStdString()<<",connected="<<p.connected<<",paired="<<p.paired;
    }

    if(handler_->webMsgIds_.contains(QString(RegisterScanStatusEvent))) {
        onSendTextMessage(buildScanStatusResponse(handler_->webMsgIds_[QString(RegisterScanStatusEvent)], 0, "success", penList));
    }
}

void WebSocketServer::handleAPStatusChangeData(APInfo apInfo)
{
    if(handler_->webMsgIds_.contains(QString(RegisterAPStatus))) {
        onSendTextMessage(buildAPStatusResponse(handler_->webMsgIds_[QString(RegisterAPStatus)], 0, "success", apInfo));
    }
}

QString WebSocketServer::buildConnectStatusResponse(QString id, int code, std::string msg, PenInfo& pen)
{
    StreamWriterBuilder wBuilder;
    Value root;
    root["id"] = id.toStdString();
    root["command"] = RegisterConnectStatusEvent;
    Value body;
    body["code"] = code;
    body["msg"] = msg;

    Value data;
    data["eventType"] = "connectStatus";

    Value eventData;
    eventData["mac"] = pen.mac.toStdString();
    eventData["name"] = pen.name.toStdString();
    eventData["connected"] = pen.connected;
    data["eventData"] = eventData;

    body["data"] = data;
    root["body"] = body;
    return QString(writeString(wBuilder, root).c_str());
}

QString WebSocketServer::buildPowerResponse(QString id, int code, std::string msg, PenInfo &pen)
{
    StreamWriterBuilder wBuilder;
    Value root;
    root["id"] = id.toStdString();
    root["command"] = RegisterPowerEvent;
    Value body;
    body["code"] = code;
    body["msg"] = msg;

    Value data;
    data["mac"] = pen.mac.toStdString();
    data["power"] = pen.battery;
    data["charging"] = pen.charging;

    body["data"] = data;
    root["body"] = body;
    return QString(writeString(wBuilder, root).c_str());
}

QString WebSocketServer::buildScanStatusResponse(QString id, int code, std::string msg, QList<PenInfo>& pens)
{
    StreamWriterBuilder wBuilder;
    Value root;
    root["id"] = id.toStdString();
    root["command"] = RegisterScanStatusEvent;
    Value body;
    body["code"] = code;
    body["msg"] = msg;

    Value data;
    data["eventType"] = "scanData";

    Value eventData;
    Json::Value penInfo;
    QList<PenInfo>::const_iterator iter = pens.cbegin();
    for (; iter != pens.cend(); ++iter)
    {
        penInfo["mac"] = (*iter).mac.toStdString();
        penInfo["name"] = (*iter).name.toStdString();
        eventData.append(penInfo);
    }
    data["eventData"] = eventData;

    body["data"] = data;
    root["body"] = body;
    return QString(writeString(wBuilder, root).c_str());
}

QString WebSocketServer::buildAPStatusResponse(QString id, int code, std::string msg, APInfo& info)
{
    StreamWriterBuilder wBuilder;
    Value root;
    root["id"] = id.toStdString();
    root["command"] = RegisterAPStatus;
    Value body;
    body["code"] = code;
    body["msg"] = msg;

    Value data;
    data["eventType"] = "ApStatus";

    Value eventData;
    eventData["ip"] = info.ip.toStdString();
    eventData["mac"] = info.mac.remove(QChar(':')).toStdString();
    eventData["status"] = info.status.toStdString();

    data["eventData"] = eventData;

    body["data"] = data;
    root["body"] = body;
    return QString(writeString(wBuilder, root).c_str());
}

QString WebSocketServer::buildAboutInfo()
{
    StreamWriterBuilder wBuilder;
    Value root;
    root["id"] = QUuid::createUuid().toString().remove("{").remove("}").toStdString();
    root["command"] = PromptAboutDialog;
    Value body;
    Value data;
    data["mainVersion"] = Config::localVersion().toStdString();
    data["charEvaluationVersion"] = ModuleInfo::getVersion(ModuleInfo::ModuleType::kCharEvaluation).toStdString();
    data["handWrittenVersion"] = ModuleInfo::getVersion(ModuleInfo::ModuleType::kHandWritten).toStdString();
    data["aipenServerVersion"] = ModuleInfo::getVersion(ModuleInfo::ModuleType::kAipenServer).toStdString();

    body["data"] = data;
    root["body"] = body;
    return QString(writeString(wBuilder, root).c_str());
}

QString WebSocketServer::buildUpgradeInfo()
{
    if(nullptr == upgradeHandle_) {
        return "";
    }
    StreamWriterBuilder wBuilder;
    Value root;
    root["id"] = QUuid::createUuid().toString().remove("{").remove("}").toStdString();
    root["command"] = PromptUpgradeDialog;
    Value body;
    Value data;
    data["force"] = upgradeHandle_->forciable();
    data["info"] = upgradeHandle_->releaseNote().toStdString();
    data["newVersion"] = upgradeHandle_->version().toStdString();

    body["data"] = data;
    root["body"] = body;
    return QString(writeString(wBuilder, root).c_str());
}

QString WebSocketServer::buildNewVersion()
{
    StreamWriterBuilder wBuilder;
    Value root;
    root["id"] = QUuid::createUuid().toString().remove("{").remove("}").toStdString();
    root["command"] = HasNewVersion;
    Value body;
    Value data;
    data["force"] = upgradeHandle_->forciable();
    data["info"] = upgradeHandle_->releaseNote().toStdString();
    data["newVersion"] = upgradeHandle_->version().toStdString();

    body["data"] = data;
    root["body"] = body;
    return QString(writeString(wBuilder, root).c_str());
}

QString WebSocketServer::buildAttendClass()
{
    StreamWriterBuilder wBuilder;
    Value root;
    root["id"] = QUuid::createUuid().toString().remove("{").remove("}").toStdString();
    root["command"] = NotifyAttendClass;
    return QString(writeString(wBuilder, root).c_str());
}

QString WebSocketServer::buildFinishClass()
{
    StreamWriterBuilder wBuilder;
    Value root;
    root["id"] = QUuid::createUuid().toString().remove("{").remove("}").toStdString();
    root["command"] = NotifyFinishClass;
    return QString(writeString(wBuilder, root).c_str());
}


