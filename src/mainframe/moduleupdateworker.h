#ifndef MODULEUPDATEWORKER_H
#define MODULEUPDATEWORKER_H

#include <QObject>
#include <QMap>

struct PackageInfo{
    QString packageMd5;
    QString packageUrl;
    QString releaseNote;
    QString localPath;
    QString newVersion;
    qint64 packageSize;
};

class ModuleUpdateWorker : public QObject
{
    Q_OBJECT
public:
    explicit ModuleUpdateWorker(QObject *parent = nullptr);

public slots:
    void onNewCharEvaluation(QString packageMd5, QString packageUrl, QString releaseNote,QString versionName,  qint64 packageSize);
    void onNewHandWritten(QString packageMd5, QString packageUrl, QString releaseNote, QString versionName, qint64 packageSize);
    void onNewAipenServer(QString packageMd5, QString packageUrl, QString releaseNote, QString versionName, qint64 packageSize);
    void onCharEvaluationDownloadFinished(QString filePath);
    void onHandWrittenDownloadFinished(QString filePath);
    void onAipenServerDownloadFinished(QString filePath);

signals:
    void requestDownloadPackage(int downloadType, const QString &url, int startPos);

private:
    void onNewPackage(int downloadType, QString packageMd5, QString packageUrl, QString releaseNote, QString versionName, qint64 packageSize);

private:
    QMap<int,PackageInfo> updateModules_;
};

#endif // MODULEUPDATEWORKER_H
