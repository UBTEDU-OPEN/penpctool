#ifndef UBTSERVER_H
#define UBTSERVER_H

#include "utilsGlobal.h"

#include <QObject>
#include <QMutex>
#include <QMap>
#include <QSet>
#include <QByteArray>
#include <QNetworkReply>

class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;

enum MyRequestType{
    kRequestAlyInfo,
    kRequestBookDetail,
    kPostEvaluateResult,
    kRequestSideXml,
    kRequestXmlDownloadUrl,
    kRequestUpgradeUrl,
    kRequestDownloadFile,
    kGetCreateWorkClass,
    kFinishAllWorkClass,
    kRequestCharEvaluationUrl,
    kRequestHandWrittenUrl,
    kRequestAipenServerUrl
};

enum DownloadType{
    kXmlFile = 0,
    kUpgradePackage,
    kCharEvaluationPackage,
    kHandWrittenPackage,
    kAipenSeverPackage
};

enum HttpFuncType {
    kHttpGet = 1,
    kHttpPost = 2
};

class UTILS_EXPORT UBTServer : public QObject
{
    Q_OBJECT
public:
    UBTServer(QObject *parent = nullptr);
    ~UBTServer();
    static UBTServer *instance();
    void requestData(const QString &url, int funcId, qulonglong tempData);
    void postData(QString url, QByteArray& data, int funcId, qulonglong tempData);
    void downloadUpgrade(int downloadType, const QString &url, int startPos);
    void downloadXml(int wordCode, const QString &url);
    static QString downloadFileUrl2DestPath(const QString &basePath, const QString &urlStr);
    static bool checkLocalUpradePackage(int downloadType, qint64 packageSize, QString packageMd5, QString packageUrl, qint64 &startPos);
    static bool checkFreeStorage(int minSize);

public slots:
    void onSaveToken(QString token);
    void stopUpgradeDownload();

signals:
    void upgradeDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void upgradeDownloadFinished(QString filePath);
    void charEvaluationDownloadFinished(QString filePath);
    void handWrittenDownloadFinished(QString filePath);
    void aipenServerDownloadFinished(QString filePath);
    void upgradeDownloadError();
    void xmlDownloadFinished(int wordCode);
    void replyResult(int funcId, int httpCode, int result, QByteArray data, qulonglong tempData);

private:
    QString deviceId();
    void onReplyFinished(QNetworkReply *reply);
    QString generateRandSignSeed();
    void setHttpHeader(QNetworkRequest &request, bool needAuthorization = true);

    void onDownloadReadyRead();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFinished();

private:
    QMutex mutex;
    QNetworkAccessManager *networkAccessManager_;
    QString deviceId_;
    QString authorization;
    QMap<QString,std::function<void(int,const QJsonObject&)>> getCallback;
    QNetworkReply* upgradeDownloadReply_;
};

#endif // UBTSERVER_H
