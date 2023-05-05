#include "localxmlworker.h"

#include <QtSql>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QDateTime>
#include <QApplication>
#include <QJsonObject>
#include <QJsonValue>
#include <QSqlError>

#include "logHelper.h"
#include "config.h"
#include "httpconfig.h"
#include "ubtserver.h"

const int kSideXmlWordCode(-1);

LocalXmlWorker *LocalXmlWorker::instance()
{
    static LocalXmlWorker* inst = new LocalXmlWorker;
    return inst;
}

QString LocalXmlWorker::getLocalXmlUpdateTime(int wordCode)
{
    QSqlQuery query;
    bool ret = query.exec(QString("select updateTime from xml_timestamp where wordCode=%1 and fileState=%2").
                          arg(wordCode).arg(FileState::Updated));
    if(ret && query.next()) {
        return query.value("updateTime").toString();
    }
    return Config::getXmlTimestamp();
}

QString LocalXmlWorker::getLocalSideXmlUpdateTime()
{
    QSqlQuery query;
    bool ret = query.exec(QString("select updateTime from xml_timestamp where wordCode=%1 and fileState=%2").
                          arg(kSideXmlWordCode).arg(FileState::Updated));
    if(ret && query.next()) {
        return query.value("updateTime").toString();
    }
    return Config::getXmlTimestamp();
}

bool LocalXmlWorker::insertLocalUpdateTime(int wordCode, QString updateTime)
{
    bool ret = deleteUpdateTime(wordCode);
    QSqlQuery query;
    ret = query.exec(QString("insert into xml_timestamp(wordCode, updateTime, fileState) values(%1,'%2', %3)").
                     arg(wordCode).arg(updateTime).arg(FileState::NotUpdated));
    LOG(INFO) << "LocalXmlWorker::insertLocalUpdateTime wordCode=" << wordCode << ",ret=" << ret;
    if(!ret) {
        LOG(WARNING) << "LocalXmlWorker::insertLocalUpdateTime wordCode=" << wordCode << "," << query.lastError().text().toStdString();
    }
    return ret;
}

bool LocalXmlWorker::deleteUpdateTime(int wordCode)
{
    QSqlQuery query;
    bool ret = query.exec(QString("delete from xml_timestamp where wordCode=%1").arg(wordCode));
//    LOG(INFO) << "LocalXmlWorker::deleteUpdateTime wordCode=" << wordCode << ",ret=" << ret;
    if(!ret) {
        LOG(WARNING) << "LocalXmlWorker::deleteUpdateTime wordCode=" << wordCode << "," << query.lastError().text().toStdString();
    }
    return ret;
}

bool LocalXmlWorker::updateFileState(int wordCode, FileState state)
{
    QSqlQuery query;
    QString sql = QString("update xml_timestamp set fileState=%1 where wordCode=%2").
            arg(state).arg(wordCode);
    bool ret = query.exec(sql);
//    LOG(INFO) << "LocalXmlWorker::setFileUpdated wordCode=" << wordCode << ",ret=" << ret;
    if(!ret) {
        LOG(WARNING) << "LocalXmlWorker::setFileUpdated wordCode=" << wordCode << "," << query.lastError().text().toStdString();
    }
    return ret;
}

LocalXmlWorker::LocalXmlWorker(QObject *parent) : QObject(parent)
{
    xmlDb_ = QSqlDatabase::addDatabase("QSQLITE");
    xmlDb_.setDatabaseName(Config::getXmlDbPath());

    if(xmlDb_.open()) {
        QSqlQuery query;
        bool ret = query.exec("select count(*) from sqlite_master where type='table' and name='xml_timestamp'");
        ret= query.next();
        if(ret) {
            int count = query.value(0).toInt();
            if(0 == count) {
                ret = query.exec("create table xml_timestamp(wordCode INTEGER PRIMARY KEY NOT NULL,"
                                       "updateTime VARCHAR(40) NOT NULL, fileState INTEGER NOT NULL)");
            }
        }
    } else {
        LOG(WARNING) << "fail open" << xmlDb_.lastError().text().toStdString();
    }
}

LocalXmlWorker::~LocalXmlWorker()
{
}

void LocalXmlWorker::installXml()
{
    QString upgradPath = Config::getXmlUpgradePath();
    QDir dir(upgradPath);
    QString targetPath = QApplication::applicationDirPath() + "/assets";
    LocalXmlWorker::FileState state = getFileState(kSideXmlWordCode);
    LOG(INFO) << "LocalXmlWorker::installXml " << upgradPath.toStdString() << ",state=" << state;
    if(dir.exists("wordmodel.zip") && LocalXmlWorker::Downloaded == state) {
        QString zipPath = upgradPath + "wordmodel.zip";
        QStringList args;
        args << "x" << QString("-o%1").arg(targetPath) << zipPath << "-aoa";
        int ret = QProcess::execute("7z.exe",args);
        LOG(INFO) << "LocalXmlWorker::installXml exact ret=" << ret;
        updateFileState(kSideXmlWordCode, FileState::Updated);
        dir.remove("wordmodel.zip");
    }

    QStringList xmlFilter;
    xmlFilter << "*.xml";
    QStringList files = dir.entryList(xmlFilter,QDir::Files);
    for(QString file : files) {
        QString fileBaseName = file.mid(0,file.size() - 4); //去除.xml的长度
        int fileCode = fileBaseName.toInt();
        QString originName = upgradPath + file;
        if(LocalXmlWorker::Downloaded == getFileState(fileCode)) {
            QString newName = targetPath + "/" + file;
            bool ret = QFile::remove(newName);
            ret = QFile::copy(originName,newName);
            updateFileState(fileCode, FileState::Updated);
        }
        QFile::remove(originName);
    }
}

LocalXmlWorker::FileState LocalXmlWorker::getFileState(int wordCode)
{
    if(xmlDb_.isOpen()) {
        QString sql = QString("select fileState from xml_timestamp where wordCode=%1").arg(wordCode);
        QSqlQuery query;
        bool ret = query.exec(sql);
        if(ret && query.next()) {
            LocalXmlWorker::FileState state = (LocalXmlWorker::FileState)query.value("fileState").toInt();
            return state;
        }
    }
    return LocalXmlWorker::Unknown;
}

void LocalXmlWorker::updateWordXml(int wordCode, QString updateTime, QString xmlUrl, bool bookDetail)
{
    QString localFormat = "yyyy-MM-ddThh:mm:ss";
    QString serverFormat = "yyyy-MM-dd hh:mm:ss";
    if(bookDetail) {
        updateTime = updateTime.mid(0,updateTime.lastIndexOf('.'));
        serverFormat = localFormat;
    }

//    LOG(INFO) << "LocalXmlWorker::updateWordXml wordCode=" << wordCode << "," << updateTime.toStdString();
    QString localTime = getLocalXmlUpdateTime(wordCode);
//    LOG(INFO) << "LocalXmlWorker::updateWordXml localTime=" << localTime.toStdString();

    QDateTime tempLocalTime = QDateTime::fromString(localTime,localFormat);
    QDateTime tempUpdateTime = QDateTime::fromString(updateTime,serverFormat);

    if(tempUpdateTime > tempLocalTime) {
        QString strUpdateTime = tempUpdateTime.toString(localFormat);
        insertLocalUpdateTime(wordCode,strUpdateTime);
        std::string url;
        url = HttpConfig::instance()->getDownloadUrl(xmlUrl.toStdString(),60);
        auto ptr = new int{wordCode};
        emit httpRequest(QString::fromStdString(url),QByteArray(),1,
                         (int)MyRequestType::kRequestXmlDownloadUrl,(qulonglong)ptr);
    }
}

void LocalXmlWorker::updateSideXml()
{
    emit httpRequest(QString::fromStdString(HttpConfig::instance()->getSideXmlUrl()),
                     QByteArray(),1,MyRequestType::kRequestSideXml,0);
}

void LocalXmlWorker::onSideXmlResponse(QString updateTime, QString url)
{
    updateTime = updateTime.mid(0,updateTime.lastIndexOf('.'));
//    LOG(INFO) << "LocalXmlWorker::onSideXmlResponse " << updateTime.toStdString();
    QString localTime = getLocalSideXmlUpdateTime();
//    LOG(INFO) << "LocalXmlWorker::onSideXmlResponse localTime=" << localTime.toStdString();
    QString format = "yyyy-MM-ddThh:mm:ss";
    QDateTime tempLocalTime = QDateTime::fromString(localTime,format);
    QDateTime tempUpdateTime = QDateTime::fromString(updateTime,format);

    if(tempUpdateTime > tempLocalTime) {
        QString strUpdateTime = tempUpdateTime.toString(format);
        insertLocalUpdateTime(kSideXmlWordCode,strUpdateTime);
        std::string fullUrl;
        fullUrl = HttpConfig::instance()->getDownloadUrl(url.toStdString(),60);
        auto ptr = new int{kSideXmlWordCode};
        emit httpRequest(QString::fromStdString(fullUrl),QByteArray(),1,
                         (int)MyRequestType::kRequestXmlDownloadUrl, (qulonglong)ptr);
    }
}

void LocalXmlWorker::onDownloadFinished(int wordCode)
{
    LOG(INFO) << "LocalXmlWorker::onDownloadFinished " << wordCode;
    updateFileState(wordCode, FileState::Downloaded);
}
