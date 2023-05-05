#ifndef UPLOADDATATHREAD_H
#define UPLOADDATATHREAD_H

#include <QThread>
#include <QMap>
#include <QRecursiveMutex>
#include <ctime>


enum ResultType{
    kEvaluationResult = 0,
    kOCRResult
};

struct UploadInfo{
    int type = kEvaluationResult;
    QString mac;
    QString imgKey;
    QString imgPath;
    QString jsonKey;
    QString jsonPath;
    QByteArray evaluateResult;
    bool imgUploaded = false;
    bool jsonUploaded = false;
    bool resultPosted = false;
    int retryTimes = 0;
    time_t lastTimeStamp;
};

class  UploadDataThread : public QThread
{
    Q_OBJECT
public:
    explicit UploadDataThread(QObject* parent = nullptr);
    ~UploadDataThread();

    void run() override;

    void onNewWord(int type, QString mac, QString imgKey, QString imgPath,
                   QString jsonKey, QString jsonPath, QByteArray evaluateResult);
    void onPostResult(int result, qulonglong tempData);

    void stopUpload();

signals:
    void httpRequest(QString,QByteArray,int,int,qulonglong);

private:
    QMap<int,UploadInfo> preUploadInfo_;
    QRecursiveMutex dataMutex_;
    volatile bool uploading = true;
    int currentIndex_;
    bool removeTempFile_ = true;
};

#endif // UPLOADDATATHREAD_H
