#ifndef UPGRADEDIALOG_H
#define UPGRADEDIALOG_H

#include <QProcess>
#include <QThread>
#include <QTime>
#include <QTimer>

enum ErrorCode {
    kUpgradeSucc = 0,
    kInsufficientSpace = 1,
    kDownloadFailed = 2,
    kUmcompressFailed = 3
};


class UmcompressWorker : public QObject
{
    Q_OBJECT

public:
    explicit UmcompressWorker(QObject *parent = nullptr);
    ~UmcompressWorker();

public slots:
    void umcompressProgram(QString packagePath);


signals:
    void umcompressFinished(int exitCode);
};

class UpgradeWorker : public QObject
{
    Q_OBJECT

public:
    explicit UpgradeWorker(bool isForced, QString packageMd5,
                           QString packageUrl, QString releaseNote,
                           QString versionName, qint64 packageSize,
                           bool firstCheck, QWidget *parent);
    ~UpgradeWorker();

    bool firstCheck() { return firstCheck_; }
    void resetFirstCheck() { firstCheck_ = false; }
    bool forciable() { return isForced_; }
    QString releaseNote() { return releaseNote_; }
    QString version() { return version_; }

public slots:
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFinished();
    void onDownloadError();
    void onUmcompressFinished(int exitCode);
    void onFinished();
    void onCancelUpgrade();
    void onStartUpgrade();


signals:
    void forceQuit();
    void upgradeCanceled();
    void downloadError();
    void requestDownloadUpgrade(int downloadType, const QString &url, int startPos);
    void startUmcompress(QString packagePath);
    void installFinished(QString newVersion);
    void downloadProgress(int percent, QByteArray strSpeed, QByteArray strRemain);
    void upgradeResult(int code, QByteArray msg);
    void startInstall();

private:
    void install();

private:
    bool isForced_ = false;
    bool firstCheck_ = false;
    QString packageMd5_;
    QString packageUrl_;
    QString releaseNote_;
    QString version_;
    qint64 packageSize_ = 0;
    QProcess *extractProcess_;
    QTime* time_ = nullptr;
    UmcompressWorker* worker_;
    QThread umcompressThread_;
    bool errorOccurred_ = false;
};

#endif // UPGRADEDIALOG_H
