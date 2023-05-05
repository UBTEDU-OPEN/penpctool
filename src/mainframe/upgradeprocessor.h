#ifndef UPGRADEPROCESSOR_H
#define UPGRADEPROCESSOR_H

#include <QObject>
#include <QJsonArray>

class UpgradeProcessor : public QObject
{
    Q_OBJECT
public:
    static UpgradeProcessor* instance();

public slots:
    void OnHandleUpgradeInfo(int result,const QJsonArray& js1);

signals:
    void newVersion(bool isForced, QString packageMd5, QString packageUrl, QString releaseNote, QString versionName,qint64 packageSize);
    void newCharEvaluation(QString packageMd5, QString packageUrl, QString releaseNote, QString versionName, qint64 packageSize);
    void newHandWritten(QString packageMd5, QString packageUrl, QString releaseNote, QString versionName, qint64 packageSize);
    void newAipenServer(QString packageMd5, QString packageUrl, QString releaseNote, QString versionName, qint64 packageSize);

private:
    explicit UpgradeProcessor(QObject *parent = nullptr);

};

#endif // UPGRADEPROCESSOR_H
