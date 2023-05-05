#include "uploadorigindataworker.h"

#include <QDir>
#include <QProcess>
#include <QDateTime>

#include "config.h"
#include "logHelper.h"
#include "httpconfig.h"
#include "alyservice.h"

UploadOriginDataWorker::UploadOriginDataWorker(QObject *parent) : QObject(parent)
{

}

void UploadOriginDataWorker::uploadOriginData(int workClassId, QString classTimeStamp)
{
    QString loginId = Config::getLoginId();
    QString tempPath = Config::getTempPath();
    QDir tempDir(tempPath);
    tempDir.mkdir("origin");
    QStringList textFilter;
    textFilter << "*.txt";
    QStringList files = tempDir.entryList(textFilter,QDir::Files);
    for(QString file : files) {
        QString originName = tempPath + file;
        QString newName = tempPath + "origin/" + file;
        QFile::copy(originName,newName);
    }
    QStringList args;
    QString fileName = QString("%1_%2_%3.zip").arg(loginId).arg(classTimeStamp).arg(workClassId);
    QString filePath = tempPath + fileName;
    args << "a" << filePath << QString("%1origin/*.txt").arg(tempPath) << "-mcu=on";
    QProcess::execute("7z.exe",args);
    QString alyKeyBase = QString::fromStdString(HttpConfig::instance()->getAlyKeyBasePath());
    QString timeStr = QDateTime::currentDateTime().toString("yyyyMMdd");
    QString zipKey = alyKeyBase + "practiceOriginPoints/" + timeStr +"/"+fileName;
    int ret = AlyService::instance()->uploadFile("ubtechinc-common-private",zipKey,filePath);
    LOG(INFO) << "AiPenManager::uploadOriginData " << zipKey.toStdString() << " upload ret=" << ret;
    QFile::remove(fileName);
    tempDir.cd("origin");
    tempDir.removeRecursively();
}
