#include "ubtserver.h"
#include "logHelper.h"
#include "cmd5.h"
#include <functional>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QByteArray>
#include <QDateTime>
#include <QHostInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDir>
#include <QJsonObject>
#include <QPluginLoader>
#include <QString>
#include <QFileInfo>
#include <QDebug>
#include <QRandomGenerator>
#include <QStorageInfo>

#include "config.h"
#include "md5.h"
#include "projectconst.h"
#include "ubtlib.h"
#include "platformport.h"

const int kHTTPRequestType = QNetworkRequest::User + 1;
const int kUserData = QNetworkRequest::User + 2;

const char* kDownloadTypeProperty = "downloadType";
const char* kWordCodeProperty = "wordCode";
const char* kDestFilePathProperty = "destFilePath";

UBTServer::UBTServer(QObject *parent)
    : QObject(parent)
    , upgradeDownloadReply_(nullptr)
{
    deviceId_ = deviceId();
    networkAccessManager_ = new QNetworkAccessManager(this);
    connect(networkAccessManager_, &QNetworkAccessManager::finished, this, &UBTServer::onReplyFinished);
}

UBTServer::~UBTServer()
{
    qDebug()<<"~UBTServer()";
    disconnect(networkAccessManager_, &QNetworkAccessManager::finished, this, &UBTServer::onReplyFinished);
}

UBTServer *UBTServer::instance()
{
    static UBTServer* inst_ = new UBTServer;
    return inst_;
}

void UBTServer::requestData(const QString &url, int funcId, qulonglong tempData)
{
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    setHttpHeader(request);
    request.setAttribute(QNetworkRequest::Attribute(kHTTPRequestType),funcId);
    request.setAttribute(QNetworkRequest::Attribute(kUserData),tempData);
    qDebug() << "requestData:" << url;
    networkAccessManager_->get(request);
}

QString UBTServer::deviceId()
{
//    QString hostName = QHostInfo::localHostName();
//    hostName = hostName.toLatin1().toHex();
//    if(hostName.size() > 10) {
//        hostName = hostName.mid(0,10);
//    }
//    hostName = "aipen-" + hostName;
//    LOG(INFO) << "hostName: " << hostName.toStdString();
    return Config::getDeviceId();
}

void UBTServer::onReplyFinished(QNetworkReply *reply)
{
//    qDebug() << "Upgrade::onReplyFinished:";
    int httpStatusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    int networkError = reply->error();
//    qDebug() << "\thttpStatusCode:" << httpStatusCode;
//    qDebug() << "\tnetworkError:" << networkError;
//    bool succ = (httpStatusCode == 200 && networkError == QNetworkReply::NoError);
    QByteArray bytes = reply->readAll();
    QString replyStr = QString::fromUtf8(bytes);
    LOG(INFO) << "\trequest url:" << reply->request().url().toString().toStdString();
    LOG(INFO) << "\treply:" << replyStr.toStdString();

    int funcId = reply->request().attribute(QNetworkRequest::Attribute(kHTTPRequestType)).toInt();

    bool replyStrEmpty = replyStr.isEmpty();
    if(MyRequestType::kRequestDownloadFile == funcId || replyStrEmpty) {
        LOG(INFO) << "UBTServer::onReplyFinished download not need process.";
        reply->deleteLater();
        return;
    }

    emit replyResult(funcId,httpStatusCode,networkError,bytes,
                     reply->request().attribute(QNetworkRequest::Attribute(kUserData)).toULongLong());
    qDebug() << "UBTServer::onReplyFinished" << reply;
    reply->deleteLater();
}

void UBTServer::downloadUpgrade(int downloadType, const QString &url, int startPos)
{
    QNetworkRequest request;

    QString folder;
    if(DownloadType::kUpgradePackage == downloadType) {
        folder = Config::getUpgradePath();
    } else if(DownloadType::kCharEvaluationPackage == downloadType) {
        folder = Config::getCharEvaluationUpgradePath();
    } else if(DownloadType::kHandWrittenPackage == downloadType) {
        folder = Config::getHandWrittenUpgradePath();
    } else if(DownloadType::kAipenSeverPackage == downloadType) {
        folder = Config::getAipenServerUpgradePath();
    } else {
        LOG(WARNING) << "UBTServer::downloadUpgrade unkown type";
        return;
    }

    auto destFilePath = UBTServer::downloadFileUrl2DestPath(folder, url);
    request.setAttribute(QNetworkRequest::Attribute(kHTTPRequestType),(int)MyRequestType::kRequestDownloadFile);
    request.setUrl(url);
    if (startPos > 0) {
        QString range = QString("bytes=%1-").arg(startPos);
        request.setRawHeader("Range", range.toLatin1());
    }
    auto reply = networkAccessManager_->get(request);
    reply->setProperty(kDownloadTypeProperty, downloadType);
    reply->setProperty(kDestFilePathProperty, destFilePath);
    connect(reply, &QNetworkReply::readyRead, this, &UBTServer::onDownloadReadyRead);
    connect(reply, &QNetworkReply::downloadProgress, this, &UBTServer::onDownloadProgress);
    connect(reply, &QNetworkReply::finished, this, &UBTServer::onDownloadFinished);
    upgradeDownloadReply_ = reply;
    qDebug() << "UBTServer::downloadUpgrade" << reply;
}

void UBTServer::downloadXml(int wordCode, const QString &url)
{
    QNetworkRequest request;
    QString folder = Config::getXmlUpgradePath();
    auto destFilePath = UBTServer::downloadFileUrl2DestPath(folder, url);
    //不支持断点续传，也无法md5校验
    if(QFile::exists(destFilePath)) {
        bool ret = QFile::remove(destFilePath);
        LOG(INFO) << "UBTServer::downloadXml remove:" << destFilePath.toStdString() << ",ret=" << ret;
    }
    request.setAttribute(QNetworkRequest::Attribute(kHTTPRequestType),(int)MyRequestType::kRequestDownloadFile);
    request.setUrl(url);
    auto reply = networkAccessManager_->get(request);
    reply->setProperty(kDownloadTypeProperty, DownloadType::kXmlFile);
    reply->setProperty(kWordCodeProperty, wordCode);
    reply->setProperty(kDestFilePathProperty, destFilePath);
    connect(reply, &QNetworkReply::readyRead, this, &UBTServer::onDownloadReadyRead);
    connect(reply, &QNetworkReply::downloadProgress, this, &UBTServer::onDownloadProgress);
    connect(reply, &QNetworkReply::finished, this, &UBTServer::onDownloadFinished);
    qDebug() << "UBTServer::downloadXml" << reply;
}

void UBTServer::stopUpgradeDownload()
{
    if(nullptr != upgradeDownloadReply_) {
        disconnect(upgradeDownloadReply_, nullptr, this, nullptr);
        upgradeDownloadReply_->abort();
        upgradeDownloadReply_->deleteLater();
        upgradeDownloadReply_ = nullptr;
    }

//    QDir upgradeFolder(Config::getUpgradePath());
//    QStringList files = upgradeFolder.entryList(QDir::Files);
//    for(QString file : files) {
//        upgradeFolder.remove(file);
//    }
}

bool UBTServer::checkLocalUpradePackage(int downloadType, qint64 packageSize, QString packageMd5, QString packageUrl, qint64 &startPos)
{
    QString dirPath;
    if(DownloadType::kUpgradePackage == downloadType) {
        dirPath = Config::getUpgradePath();
    } else if(DownloadType::kCharEvaluationPackage == downloadType) {
        dirPath = Config::getCharEvaluationUpgradePath();
    } else if(DownloadType::kHandWrittenPackage == downloadType) {
        dirPath = Config::getHandWrittenUpgradePath();
    } else if(DownloadType::kAipenSeverPackage == downloadType) {
        dirPath = Config::getAipenServerUpgradePath();
    }

    auto destFilePath = UBTServer::downloadFileUrl2DestPath(dirPath,packageUrl);
    QFile destFile(destFilePath);
    if (destFile.exists()) {
        if (destFile.size() < packageSize) {
            startPos = destFile.size();
            return true;
        } else if (destFile.size() == packageSize) {
            if (MD5::fileMd5(destFilePath) == packageMd5) {
                startPos = packageSize;
                return false; // no need to download
            } else {
                LOG(WARNING) << "md5 of local package is not the same as remote package";
                destFile.remove();
                startPos = 0;
                return true;
            }
        } else {
            LOG(WARNING) << "local file is bigger than remote package";
            destFile.remove();
            startPos = 0;
            return true;
        }
    } else {
        startPos = 0;
        return true;
    }
}

QString UBTServer::downloadFileUrl2DestPath(const QString &basePath, const QString &urlStr)
{
    auto url = QUrl(urlStr);
    auto destFilePath = basePath;
    if(!destFilePath.endsWith('/')) {
        destFilePath += '/';
    }
    destFilePath += url.fileName();
    return destFilePath;
}

void UBTServer::onSaveToken(QString token)
{
    LOG(INFO) << "UBTServer::onSaveToken " << token.toStdString();
    this->authorization = token;
}

void UBTServer::onDownloadReadyRead()
{
    if (auto downloadReply = dynamic_cast<QNetworkReply*>(sender())) {
        auto destFilePath = downloadReply->property(kDestFilePathProperty).toString();
        if (destFilePath.isEmpty()) {
            LOG(ERROR) << "dest file is empty";
            return;
        }
        QFileInfo destFileInfo(destFilePath);
        if (!destFileInfo.dir().exists()) {
            destFileInfo.dir().mkdir(".");
        }
        QFile destFile(destFilePath);
        if (destFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
            destFile.write(downloadReply->readAll());
        }
        destFile.close();
    } else {
        LOG(ERROR) << "sender is not QNetworkRelpy";
    }
}

void UBTServer::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (auto downloadReply = dynamic_cast<QNetworkReply*>(sender())) {
        auto downloadType = downloadReply->property(kDownloadTypeProperty).toInt();
        if(DownloadType::kUpgradePackage == downloadType) {
            emit upgradeDownloadProgress(bytesReceived, bytesTotal);
        }

    } else {
        LOG(ERROR) << "sender is not QNetworkRelpy";
    }
//    qDebug() << "Upgrade::onDownloadProgress " << bytesReceived << " " << bytesTotal;
}

void UBTServer::onDownloadFinished()
{
    if (auto downloadReply = dynamic_cast<QNetworkReply*>(sender())) {
        LOG(WARNING) << downloadReply->errorString().toStdString() << downloadReply->error();
        auto downloadType = downloadReply->property(kDownloadTypeProperty).toInt();
        if(QNetworkReply::NoError == downloadReply->error()) {
            auto filePath = downloadReply->property(kDestFilePathProperty).toString();
            if(DownloadType::kUpgradePackage == downloadType) {
                emit upgradeDownloadFinished(filePath);
            } else if(DownloadType::kXmlFile == downloadType) {
                auto wordCode = downloadReply->property(kWordCodeProperty).toInt();
                emit xmlDownloadFinished(wordCode);
            } else if(DownloadType::kCharEvaluationPackage == downloadType) {
                emit charEvaluationDownloadFinished(filePath);
            } else if(DownloadType::kHandWrittenPackage == downloadType) {
                emit handWrittenDownloadFinished(filePath);
            } else if(DownloadType::kAipenSeverPackage == downloadType) {
                emit aipenServerDownloadFinished(filePath);
            }
        } else {
            if(DownloadType::kUpgradePackage == downloadType) {
                emit upgradeDownloadError();
            }
        }
        downloadReply->deleteLater();
        if(DownloadType::kUpgradePackage == downloadType) {
            upgradeDownloadReply_ = nullptr;
        }

    } else {
        LOG(ERROR) << "sender is not QNetworkRelpy";
    }
}

void UBTServer::setHttpHeader(QNetworkRequest &request, bool needAuthorization)
{
    std::string sign = getHeaderXUBTSignV3(deviceId_.toStdString(),APP_KEY,APP_ID);
    qDebug() << "Upgrade::setHttpHeader:";
    qDebug() << "\tsign:" << QString::fromStdString(sign);
    qDebug() << "\tdeviceId:" << deviceId_;
    qDebug() << "\tAuthorization:" << authorization;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json;charset=UTF-8");
    request.setRawHeader("X-UBT-Sign", QByteArray::fromStdString(sign));
    request.setRawHeader("X-UBT-AppId", QByteArray::fromStdString(APP_ID));
    request.setRawHeader("X-UBT-DeviceId", QByteArray::fromStdString(deviceId_.toStdString()));
    if (needAuthorization) {
        request.setRawHeader("authorization", QByteArray::fromStdString(authorization.toStdString()));
    }
    LOG(INFO) << "UBTServer::setHttpHeader authorization" << authorization.toStdString();
}

void UBTServer::postData(QString url, QByteArray& data, int funcId, qulonglong tempData)
{
//    auto url = Settings::upgradeUrl() + "group/upgradable" +
//               "?productName=" + productName_ +
//               "&groupVersion=" + groupVersion /*+
//               "&languageName=" + languageName*/;
    qDebug() << "Upgrade::requestGroupUpgradable url=" << url;
//    getCallback.insert(url,callback);

    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setAttribute(QNetworkRequest::Attribute(kHTTPRequestType),funcId);
    request.setAttribute(QNetworkRequest::Attribute(kUserData),tempData);
    setHttpHeader(request);
    networkAccessManager_->post(request,data);
}

bool UBTServer::checkFreeStorage(int minSize)
{
    QStorageInfo info(Config::getAipenPath());
    int freeSize = static_cast<int>(info.bytesFree() / kMill);
    return freeSize > minSize;
}
