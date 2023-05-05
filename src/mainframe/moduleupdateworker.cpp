#include "moduleupdateworker.h"

#include <QProcess>
#include <QDir>
#include <QUrl>
#include <QFileInfo>

#include "ubtserver.h"
#include "projectconst.h"
#include "config.h"
#include "logHelper.h"
#include "md5.h"

ModuleUpdateWorker::ModuleUpdateWorker(QObject *parent) : QObject(parent)
{

}

void ModuleUpdateWorker::onNewCharEvaluation(QString packageMd5, QString packageUrl, QString releaseNote, QString versionName, qint64 packageSize)
{
    onNewPackage((int)DownloadType::kCharEvaluationPackage,packageMd5,packageUrl,releaseNote,versionName,packageSize);
}

void ModuleUpdateWorker::onNewHandWritten(QString packageMd5, QString packageUrl, QString releaseNote, QString versionName, qint64 packageSize)
{
    onNewPackage((int)DownloadType::kHandWrittenPackage,packageMd5,packageUrl,releaseNote,versionName,packageSize);
}

void ModuleUpdateWorker::onNewAipenServer(QString packageMd5, QString packageUrl, QString releaseNote, QString versionName, qint64 packageSize)
{
    onNewPackage((int)DownloadType::kAipenSeverPackage,packageMd5,packageUrl,releaseNote,versionName,packageSize);
}

void ModuleUpdateWorker::onCharEvaluationDownloadFinished(QString filePath)
{
    LOG(INFO) << "ModuleUpdateWorker::onCharEvaluationDownloadFinished";
    if(!updateModules_.contains(DownloadType::kCharEvaluationPackage)) {
        LOG(WARNING) << "ModuleUpdateWorker::onCharEvaluationDownloadFinished wrong finished.";
        return;
    }
    Config::removeCharEvaluationDownloadingPackage();
    if (MD5::fileMd5(filePath) == updateModules_[DownloadType::kCharEvaluationPackage].packageMd5) {
        Config::saveCharEvaluationUpdatePackage(filePath,updateModules_[DownloadType::kCharEvaluationPackage].newVersion);
    } else {
        LOG(WARNING) << "ModuleUpdateWorker::onCharEvaluationDownloadFinished md5 unmatched "
                     << filePath.toStdString();
        QFile::remove(filePath);
    }
}

void ModuleUpdateWorker::onHandWrittenDownloadFinished(QString filePath)
{
    LOG(INFO) << "ModuleUpdateWorker::onHandWrittenDownloadFinished";
    if(!updateModules_.contains(DownloadType::kHandWrittenPackage)) {
        LOG(WARNING) << "ModuleUpdateWorker::onHandWrittenDownloadFinished wrong finished.";
        return;
    }
    Config::removeHandWrittenDownloadingPackage();
    if (MD5::fileMd5(filePath) == updateModules_[DownloadType::kHandWrittenPackage].packageMd5) {
        Config::saveHandWrittenUpdatePackage(filePath,updateModules_[DownloadType::kHandWrittenPackage].newVersion);
    } else {
        LOG(WARNING) << "ModuleUpdateWorker::onHandWrittenDownloadFinished md5 unmatched "
                     << filePath.toStdString();
        QFile::remove(filePath);
    }
}

void ModuleUpdateWorker::onAipenServerDownloadFinished(QString filePath)
{
    LOG(INFO) << "ModuleUpdateWorker::onAipenServerDownloadFinished";
    if(!updateModules_.contains(DownloadType::kAipenSeverPackage)) {
        LOG(WARNING) << "ModuleUpdateWorker::onAipenServerDownloadFinished wrong finished.";
        return;
    }
    Config::removeAipenServerDownloadingPackage();
    if (MD5::fileMd5(filePath) == updateModules_[DownloadType::kAipenSeverPackage].packageMd5) {
        Config::saveAipenServerUpdatePackage(filePath,updateModules_[DownloadType::kAipenSeverPackage].newVersion);
    } else {
        LOG(WARNING) << "ModuleUpdateWorker::onAipenServerDownloadFinished md5 unmatched "
                     << filePath.toStdString();
        QFile::remove(filePath);
    }
}

void ModuleUpdateWorker::onNewPackage(int downloadType, QString packageMd5, QString packageUrl,
                                      QString releaseNote, QString versionName, qint64 packageSize)
{
    QString dirPath;
    if(DownloadType::kCharEvaluationPackage == downloadType) {
        dirPath = Config::getCharEvaluationUpgradePath();
    } else if(DownloadType::kHandWrittenPackage == downloadType) {
        dirPath = Config::getHandWrittenUpgradePath();
    } else if(DownloadType::kAipenSeverPackage == downloadType) {
        dirPath = Config::getAipenServerUpgradePath();
    } else {
        LOG(WARNING) << "UBTServer::checkLocalUpradePackage unkown type";
        return;
    }

    auto destFilePath = UBTServer::downloadFileUrl2DestPath(dirPath,packageUrl);
    PackageInfo info;
    info.localPath = destFilePath;
    info.packageMd5 = packageMd5;
    info.packageSize = packageSize;
    info.packageUrl = packageUrl;
    info.newVersion = versionName;
    info.releaseNote = releaseNote;

    updateModules_.insert(downloadType,info);

    qint64 startPos = 0;
    if (UBTServer::checkLocalUpradePackage(downloadType,packageSize,packageMd5,packageUrl,startPos)) {
        qint64 requireSize = packageSize - startPos;
        int requireSizeMB = static_cast<int>(requireSize/kMill);
        ++requireSizeMB; //简单处理，直接+1
        if(UBTServer::checkFreeStorage(requireSizeMB)) {
            if(DownloadType::kCharEvaluationPackage == downloadType) {
                QString oldPackage = Config::charEvaluationDownloadingPackage();
                if(!oldPackage.isEmpty() && oldPackage != destFilePath) {
                    QFile::remove(oldPackage);
                }
                Config::saveCharEvaluationDownloadingPackage(destFilePath);
            } else if(DownloadType::kHandWrittenPackage == downloadType) {
                QString oldPackage = Config::handWrittenDownloadingPackage();
                if(!oldPackage.isEmpty() && oldPackage != destFilePath) {
                    QFile::remove(oldPackage);
                }
                Config::saveHandWrittenDownloadingPackage(destFilePath);
            } else if(DownloadType::kAipenSeverPackage == downloadType) {
                QString oldPackage = Config::aipenServerDownloadingPackage();
                if(!oldPackage.isEmpty() && oldPackage != destFilePath) {
                    QFile::remove(oldPackage);
                }
                Config::saveAipenServerDownloadingPackage(destFilePath);
            }
            emit requestDownloadPackage(downloadType,packageUrl,startPos);
        }
    } else {
        if(DownloadType::kCharEvaluationPackage == downloadType) {
            Config::removeCharEvaluationDownloadingPackage();
            Config::saveCharEvaluationUpdatePackage(destFilePath,versionName);
        } else if(DownloadType::kHandWrittenPackage == downloadType) {
            Config::removeHandWrittenDownloadingPackage();
            Config::saveHandWrittenUpdatePackage(destFilePath,versionName);
        } else if(DownloadType::kAipenSeverPackage == downloadType) {
            Config::removeAipenServerDownloadingPackage();
            Config::saveAipenServerUpdatePackage(destFilePath,versionName);
        }
    }
}


