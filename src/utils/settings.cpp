#include "settings.h"
#include "logHelper.h"
#include "fileDirHandler.h"

#include <QSettings>
#include <QDir>
#include <QStringList>
#include <QByteArray>

const QString Settings::FOLDER_SETTINGS                 = "../../configs";
const QString Settings::FOLDER_DEFAULT_PLUGIN           = "../../plugins";
const QString Settings::FOLDER_TEMP                     = "temp";
const QString Settings::FOLDER_CLASSROOM                = "../../classroom";
const QString Settings::FOLDER_SHARED                   = "shared";
const QString Settings::FOLDER_DEVICE_UPGRADE_PACKAGE   = "upgrade";

const QString Settings::FILE_SETTINGS                   = "settings.ini";
const QString Settings::FILE_OPENED_CLASSROOMS_RECORD   = "openedClassrooms";

const QString Settings::SECTION_LOG                     = "log";
const QString Settings::SECTION_PLUGIN                  = "plugin";
const QString Settings::SECTION_UPGRADE                 = "upgrade";
const QString Settings::SECTION_DEVICE_UPGRADE          = "device_upgrade";
const QString Settings::SECTION_USER                    = "user";

const QString Settings::KEY_LOG_PRINT_SERERITY          = "print_serverity";
const QString Settings::KEY_LOG_FILE_SERERITY           = "file_serverity";
const QString Settings::KEY_FOLDER_PATH                 = "folder_path";
const QString Settings::KEY_CLASSROOM                   = "classroom";
const QString Settings::KEY_IGNORE_GROUP                = "ignore_group";
const QString Settings::KEY_UPGRADE_FOLDER              = "upgrade_folder";
const QString Settings::KEY_URL                         = "url";

const QString Settings::KEY_UPGRADE_TIME_LIMIT                          = "upgrade_time_limit";
const QString Settings::KEY_TRANSFER_ATTEMPTION_TIMES_LIMIT             = "transfer_attemption_times_limit";
const QString Settings::KEY_TRANSFER_CONTINUOUS_ATTEMPTION_TIMES_LIMIT  = "transfer_continuous_attemption_times_limit";
const QString Settings::KEY_PARALLEL_TRANSFER_DEVICE_COUNT_LIMIT        = "parallel_transfer_device_count_limit";

const int Settings::VALUE_LOG_PRINT_DEFAULT_SERVERITY       = 0; // 0 - info; 1 - warning; 2 - error; 3 - fatal
const int Settings::VALUE_LOG_FILE_DEFAULT_SERVERITY        = 2;  // 0 - info; 1 - warning; 2 - error; 3 - fatal
const QString Settings::VALUE_LOG_FOLDER_DEFAULT_PATH       = "log";
const QString Settings::VALUE_PLUGIN_FOLDER_DEFAULT_PATH    = "../../plugins";
const QString Settings::VALUE_PLUGIN_RES_FOLDER_NAME        = "res";
const QString Settings::VALUE_UPGRADE_FOLDER_DEFAULT_PATH   = "../../upgrade";

const int Settings::VALUE_UPGRADE_TIME_LIMIT                            = 5400;
const int Settings::VALUE_TRANSFER_ATTEMPTION_TIMES_LIMIT               = 300;
const int Settings::VALUE_PARALLEL_TRANSFER_DEVICE_COUNT_LIMIT          = 5;
const int Settings::VALUE_TRANSFER_CONTINUOUS_ATTEMPTION_TIMES_LIMIT    = 60;


QString Settings::settingsAbsPath()
{
    QString settingsFilePath = FOLDER_SETTINGS + "/" + FILE_SETTINGS;
    settingsFilePath = FileDirHandler::absolutePath(settingsFilePath);
    return settingsFilePath;
}

int Settings::getLogPrintServerity()
{
    QSettings settings(settingsAbsPath(), QSettings::IniFormat);
    settings.beginGroup(SECTION_LOG);
    int serverity = settings.value(KEY_LOG_PRINT_SERERITY, VALUE_LOG_PRINT_DEFAULT_SERVERITY).toInt();
    settings.endGroup();
    return serverity;
}

void Settings::saveLogPrintServerity(int serverity)
{
    QSettings settings(settingsAbsPath(), QSettings::IniFormat);
    settings.beginGroup(SECTION_LOG);
    settings.setValue(KEY_LOG_PRINT_SERERITY, serverity);
    settings.endGroup();
    settings.sync();
}

int Settings::getLogFileServerity()
{
    QSettings settings(settingsAbsPath(), QSettings::IniFormat);
    settings.beginGroup(SECTION_LOG);
    int serverity = settings.value(KEY_LOG_FILE_SERERITY, VALUE_LOG_FILE_DEFAULT_SERVERITY).toInt();
    settings.endGroup();
    return serverity;
}

void Settings::saveLogFileServerity(int serverity)
{
    QSettings settings(settingsAbsPath(), QSettings::IniFormat);
    settings.beginGroup(SECTION_LOG);
    settings.setValue(KEY_LOG_FILE_SERERITY, serverity);
    settings.endGroup();
    settings.sync();
}

QString Settings::logFolderAbsPath()
{
    QSettings settings(settingsAbsPath(), QSettings::IniFormat);
    settings.beginGroup(SECTION_LOG);
    QString logFolderPath = settings.value(KEY_FOLDER_PATH, VALUE_LOG_FOLDER_DEFAULT_PATH).toString();
    logFolderPath = FileDirHandler::absolutePath(logFolderPath);
    settings.endGroup();
    return logFolderPath;
}

void Settings::saveLogFolderPath(const QString &path)
{
    QSettings settings(settingsAbsPath(), QSettings::IniFormat);
    settings.beginGroup(SECTION_LOG);
    settings.setValue(KEY_FOLDER_PATH, path);
    settings.endGroup();
    settings.sync();
}

QString Settings::pluginsAbsPath()
{
    QSettings settings(settingsAbsPath(), QSettings::IniFormat);
    settings.beginGroup(SECTION_PLUGIN);
    QString pluginFolderPath = settings.value(KEY_FOLDER_PATH, VALUE_PLUGIN_FOLDER_DEFAULT_PATH).toString();
    pluginFolderPath = FileDirHandler::absolutePath(pluginFolderPath);
    settings.endGroup();
    return pluginFolderPath;
}

void Settings::savePluginPath(const QString &path)
{
    QSettings settings(settingsAbsPath(), QSettings::IniFormat);
    settings.beginGroup(SECTION_PLUGIN);
    settings.setValue(KEY_FOLDER_PATH, path);
    settings.endGroup();
    settings.sync();
}

QString Settings::defaultClassroomFileAbsDir()
{
    QString classroomFolderPath = FOLDER_CLASSROOM;
    classroomFolderPath = FileDirHandler::absolutePath(classroomFolderPath);
    return classroomFolderPath;
}

QString Settings::ignoreGroupVersion()
{
    QSettings settings(settingsAbsPath(), QSettings::IniFormat);
    settings.beginGroup(SECTION_UPGRADE);
    QString ignore = settings.value(KEY_IGNORE_GROUP).toString();
    settings.endGroup();
    return ignore;
}

void Settings::saveIgnoreGroupVersion(const QString &version)
{
    QSettings settings(settingsAbsPath(), QSettings::IniFormat);
    settings.beginGroup(SECTION_UPGRADE);
    settings.setValue(KEY_IGNORE_GROUP, version);
    settings.endGroup();
    settings.sync();
}

QString Settings::upgradeUrl()
{
    QSettings settings(settingsAbsPath(), QSettings::IniFormat);
    settings.beginGroup(SECTION_UPGRADE);
    QString upgradeUrl = settings.value(KEY_URL).toString();
    settings.endGroup();
    return upgradeUrl;
}

QString Settings::upgradeFolderAbsPath()
{
    QSettings settings(settingsAbsPath(), QSettings::IniFormat);
    settings.beginGroup(SECTION_UPGRADE);
    auto upgradeFolderPath = settings.value(KEY_UPGRADE_FOLDER, VALUE_UPGRADE_FOLDER_DEFAULT_PATH).toString();
    upgradeFolderPath = FileDirHandler::absolutePath(upgradeFolderPath);
    settings.endGroup();
    return upgradeFolderPath;
}

QStringList Settings::openedClassroomsRecord()
{
    QStringList openedClassroomsPathes;

    QString recordFilePath = FOLDER_CLASSROOM + "/" + FILE_OPENED_CLASSROOMS_RECORD;
    recordFilePath = FileDirHandler::absolutePath(recordFilePath);
    QFile recordFile(recordFilePath);
    if (recordFile.open(QFile::ReadOnly)) {
        auto classroomFilePathes = QString::fromUtf8(recordFile.readAll());
        if (!classroomFilePathes.isEmpty()) {
            openedClassroomsPathes = classroomFilePathes.split("\n");
        }
        recordFile.close();
    }

    return openedClassroomsPathes;
}

void Settings::saveOpenedClassroomsRecord(const QStringList &openedClassroomsPathes)
{
    QString recordFilePath = FOLDER_CLASSROOM + "/" + FILE_OPENED_CLASSROOMS_RECORD;
    recordFilePath = FileDirHandler::absolutePath(recordFilePath);

    QFile recordFile(recordFilePath);
    if (recordFile.open(QFile::ReadWrite | QFile::Truncate)) {
        recordFile.write(openedClassroomsPathes.join("\n").toUtf8());
        recordFile.close();
    }
}

QString Settings::sharedFolderAbsPath()
{
    QString sharedFolderPath = FOLDER_SHARED;
    sharedFolderPath = FileDirHandler::absolutePath(sharedFolderPath);
    return sharedFolderPath;
}

QString Settings::deviceUpgradeFolderAbsPath(const QString &deviceType)
{
    QString upgradeFolderPath = FOLDER_DEVICE_UPGRADE_PACKAGE + QDir::separator() + deviceType;
    upgradeFolderPath = FileDirHandler::absolutePath(upgradeFolderPath);
    return upgradeFolderPath;
}

int Settings::upgradeTimeLimit()
{
    QSettings settings(settingsAbsPath(), QSettings::IniFormat);
    settings.beginGroup(SECTION_DEVICE_UPGRADE);
    int limit = settings.value(KEY_UPGRADE_TIME_LIMIT, VALUE_UPGRADE_TIME_LIMIT).toInt();
    settings.endGroup();
    return limit;
}

int Settings::transferAttemptionTimesLimit()
{
    QSettings settings(settingsAbsPath(), QSettings::IniFormat);
    settings.beginGroup(SECTION_DEVICE_UPGRADE);
    int limit = settings.value(KEY_TRANSFER_ATTEMPTION_TIMES_LIMIT, VALUE_TRANSFER_ATTEMPTION_TIMES_LIMIT).toInt();
    settings.endGroup();
    return limit;
}

int Settings::parallelTransferDeviceCountLimit()
{
    QSettings settings(settingsAbsPath(), QSettings::IniFormat);
    settings.beginGroup(SECTION_DEVICE_UPGRADE);
    int limit = settings.value(KEY_PARALLEL_TRANSFER_DEVICE_COUNT_LIMIT, VALUE_PARALLEL_TRANSFER_DEVICE_COUNT_LIMIT).toInt();
    settings.endGroup();
    return limit;
}

int Settings::transferContinuousAttemptionTimesLimit()
{
    QSettings settings(settingsAbsPath(), QSettings::IniFormat);
    settings.beginGroup(SECTION_DEVICE_UPGRADE);
    int limit = settings.value(KEY_TRANSFER_CONTINUOUS_ATTEMPTION_TIMES_LIMIT, VALUE_TRANSFER_CONTINUOUS_ATTEMPTION_TIMES_LIMIT).toInt();
    settings.endGroup();
    return limit;
}



