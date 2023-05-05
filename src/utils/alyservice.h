#ifndef ALYSERVICE_H
#define ALYSERVICE_H

#include <QObject>
#include "utilsGlobal.h"
#include "QJsonObject"
#include <QTimer>
#include <QMutex>
#include <atomic>

namespace AlibabaCloud {
namespace OSS {
    class OssClient;
}
}

class UTILS_EXPORT AlyService : public QObject
{
    Q_OBJECT
public:

    static AlyService* instance();

    explicit AlyService(QObject *parent = nullptr);
    void init();
    int uploadFile(const QString& BucketName,const QString& fileKey,const QString& filePath);
    int downloadFile(const QString& BucketName,const QString& fileKey,const QString& filePath);
    ~AlyService();

    void onAlyInfo(int result,const QJsonObject& js1);

    void onTimeout();

    void onStopTimer();

private:
    AlibabaCloud::OSS::OssClient *client;
    QTimer* timer_;
    QMutex clientMutex_;
    void requestToken();
    void restartTimer(bool clientInitResult);
    std::atomic<bool> requested_{false};

signals:
    void httpRequest(QString,QByteArray,int, int,qulonglong);
};

#endif // ALYSERVICE_H
