#include "upgradeworker.h"
#include "ui_upgradedialog.h"

#include <QDebug>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QPainter>
#include <QStyle>
#include <QScrollBar>
#include <QStorageInfo>
#include <QApplication>

#include "ubtserver.h"
#include "logHelper.h"
#include "md5.h"
#include "config.h"
#include "projectconst.h"


UpgradeWorker::UpgradeWorker(bool isForced, QString packageMd5,
                             QString packageUrl, QString releaseNote,
                             QString versionName, qint64 packageSize,
                             bool firstCheck, QWidget *parent) :
    QObject(parent),
    isForced_(isForced),
    packageMd5_(packageMd5),
    packageUrl_(packageUrl),
    releaseNote_(releaseNote),
    packageSize_(packageSize),
    time_(nullptr),
    firstCheck_(firstCheck),
    version_(versionName)
{
}

UpgradeWorker::~UpgradeWorker()
{
    umcompressThread_.quit();
    umcompressThread_.wait();
}

void UpgradeWorker::onCancelUpgrade()
{
    emit upgradeCanceled();
    if(isForced_) {
        emit forceQuit();
    }
}

void UpgradeWorker::onStartUpgrade()
{
    qint64 startPos = 0;
    if (UBTServer::checkLocalUpradePackage(DownloadType::kUpgradePackage,
                                           packageSize_,packageMd5_,
                                           packageUrl_,startPos)) {
        qint64 requireSize = packageSize_ - startPos;
        int requireSizeMB = static_cast<int>(requireSize/kMill);
        ++requireSizeMB; //简单处理，直接+1
        if(UBTServer::checkFreeStorage(requireSizeMB)) {
            emit requestDownloadUpgrade((int)DownloadType::kUpgradePackage,packageUrl_,startPos);
            time_ = new QTime;
            time_->start();
        } else {
            emit upgradeResult(static_cast<int>(kInsufficientSpace),
                               QString::fromLocal8Bit("磁盘空间已满，请清理后再重新运行").toUtf8());
        }
    } else {
        install();
    }
}

void UpgradeWorker::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    int progress = (bytesReceived+packageSize_-bytesTotal) * 100 / packageSize_;
    int elapsedTime = qRound(time_->elapsed()/1000.0);
    if(0 == elapsedTime) {
        elapsedTime = 1;
    }
    int speed = bytesReceived/elapsedTime;
    if(speed == 0) {
        LOG(WARNING) << "bytesReceived:" << bytesReceived << ",elapsedTime:" << elapsedTime;
        return;
    }
    QString speedStr;
    if(speed < kKilo) {
        speedStr = QString::fromLocal8Bit("%1B/s").arg(speed);
    } else if(kKilo <= speed && speed < kMill) {
        speedStr = QString::fromLocal8Bit("%1K/s").arg(speed/kKilo);
    } else if(speed >= kMill) {
        speedStr = QString::fromLocal8Bit("%1M/s").arg(speed/kMill);
    }

    qint64 remain = bytesTotal - bytesReceived;
    int remainSec = remain / speed;

    int sec = remainSec % 60;
    int min = (remainSec / 60) % 60;
    QString remainStr;
    if(min != 0) {
        remainStr = QString::fromLocal8Bit("%1分%2秒").arg(min).arg(sec);
    } else {
        remainStr = QString::fromLocal8Bit("%1秒").arg(sec);
    }
    emit downloadProgress(progress,speedStr.toUtf8(),remainStr.toUtf8());
}

void UpgradeWorker::onDownloadFinished()
{
    LOG(INFO) << "UpgradeWorker::onDownloadFinished";
    install();
}

void UpgradeWorker::onDownloadError()
{
    errorOccurred_ = true;
    emit upgradeResult(static_cast<int>(kDownloadFailed),
                       QString::fromLocal8Bit("当前网络环境异常，请检查后重新运行").toUtf8());
}

void UpgradeWorker::onUmcompressFinished(int exitCode)
{
    worker_->deleteLater();
    if(0 == exitCode) {
        emit upgradeResult(0,"");
    } else {
        errorOccurred_ = true;
        emit upgradeResult(1,QString::fromLocal8Bit("解压缩失败，请检查压缩包是否正确！").toUtf8());
    }
}


void UpgradeWorker::onFinished()
{
    QUrl url(packageUrl_);
    QString fileName = url.fileName();
    fileName = fileName.mid(0,fileName.lastIndexOf('.'));
    emit installFinished(fileName);
}


void UpgradeWorker::install()
{
    LOG(INFO) << "UpgradeWorker::install";
    emit startInstall();
    if(!UBTServer::checkFreeStorage(Config::getMinFreeSize())) {
        errorOccurred_ = true;
        emit upgradeResult(1,QString::fromLocal8Bit("磁盘空间已满，请清理后再重新运行").toUtf8());
        return;
    }

    QUrl pkgUrl(packageUrl_);
    QString packagePath = Config::getUpgradePath() + pkgUrl.fileName();

    worker_ = new UmcompressWorker;
    worker_->moveToThread(&umcompressThread_);
    connect(worker_,&UmcompressWorker::umcompressFinished,this,&UpgradeWorker::onUmcompressFinished);
    connect(this,&UpgradeWorker::startUmcompress,worker_,&UmcompressWorker::umcompressProgram);
    umcompressThread_.start();
    emit startUmcompress(packagePath);
}



UmcompressWorker::UmcompressWorker(QObject *parent)
    : QObject(parent)
{

}

UmcompressWorker::~UmcompressWorker()
{

}

void UmcompressWorker::umcompressProgram(QString packagePath)
{
    QString installPath = "../";
    QDir installDir(installPath);
    QString extractCmd = "7z.exe";
    QStringList extractArguments;
    extractArguments.append("x");
    extractArguments.append("-o" + installDir.absolutePath());
    extractArguments.append(packagePath);
    extractArguments.append("-aoa");
    QProcess extractProcess;
    extractProcess.start(extractCmd, extractArguments);
    extractProcess.waitForFinished(-1);
    int exitCode = extractProcess.exitCode();
//    QFile::remove(packagePath);
    QDir upgradeFolder(Config::getUpgradePath());
    QStringList files = upgradeFolder.entryList(QDir::Files);
    for(QString file : files) {
        upgradeFolder.remove(file);
    }
    emit umcompressFinished(exitCode);
}

