#ifndef HTTPWORKER_H
#define HTTPWORKER_H

#include <QObject>
#include "utilsGlobal.h"

class UTILS_EXPORT HttpWorker : public QObject
{
    Q_OBJECT
public:
    explicit HttpWorker(QObject *parent = nullptr);

public slots:
    void onHttpRequest(QString url, QByteArray data, int type, int funcId, qulonglong tempData);
    void onReplyResult(int funcId, int httpCode, int result, QByteArray data, qulonglong tempData);
    void onRequestDownloadUpgrade(int downloadType, const QString &url, int startPos);
    void onStopTimer();

signals:
    void postResult(int result, qulonglong tempData);
    void sideXmlResponse(QString updateTime, QString url);
    void sigGetBookDetail(int result,const QJsonObject& js1, qulonglong tempData);
    void sigGetWorkClassId(int result,const QJsonObject& js1, qulonglong tempData);
    void sigHandleUpgradeInfo(int result,const QJsonArray& js1);

};

#endif // HTTPWORKER_H
