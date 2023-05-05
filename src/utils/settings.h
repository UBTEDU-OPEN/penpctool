#ifndef SETTINGS_H
#define SETTINGS_H

#include "utilsGlobal.h"

#include <QString>

class UTILS_EXPORT Settings
{
public:
    // folder
    static const QString FOLDER_SETTINGS;
    static const QString FOLDER_DEFAULT_PLUGIN;
    static const QString FOLDER_TEMP;
    static const QString FOLDER_CLASSROOM;
    static const QString FOLDER_SHARED;
    static const QString FOLDER_DEVICE_UPGRADE_PACKAGE;
    // file
    static const QString FILE_SETTINGS;
    static const QString FILE_OPENED_CLASSROOMS_RECORD;
    // section
    static const QString SECTION_LOG;
    static const QString SECTION_PLUGIN;
    static const QString SECTION_UPGRADE;
    static const QString SECTION_DEVICE_UPGRADE;
    static const QString SECTION_USER;
    // key
    static const QString KEY_LOG_PRINT_SERERITY;
    static const QString KEY_LOG_FILE_SERERITY;
    static const QString KEY_FOLDER_PATH;
    static const QString KEY_CLASSROOM;
    static const QString KEY_IGNORE_GROUP;
    static const QString KEY_UPGRADE_FOLDER;
    static const QString KEY_URL;
    static const QString KEY_UPGRADE_TIME_LIMIT;
    static const QString KEY_TRANSFER_ATTEMPTION_TIMES_LIMIT;
    static const QString KEY_PARALLEL_TRANSFER_DEVICE_COUNT_LIMIT;
    static const QString KEY_TRANSFER_CONTINUOUS_ATTEMPTION_TIMES_LIMIT;
    // value
    static const int VALUE_LOG_PRINT_DEFAULT_SERVERITY;
    static const int VALUE_LOG_FILE_DEFAULT_SERVERITY;
    static const QString VALUE_LOG_FOLDER_DEFAULT_PATH;
    static const QString VALUE_PLUGIN_FOLDER_DEFAULT_PATH;
    static const QString VALUE_PLUGIN_RES_FOLDER_NAME;
    static const QString VALUE_UPGRADE_FOLDER_DEFAULT_PATH;
    static const int VALUE_UPGRADE_TIME_LIMIT;
    static const int VALUE_TRANSFER_ATTEMPTION_TIMES_LIMIT;
    static const int VALUE_PARALLEL_TRANSFER_DEVICE_COUNT_LIMIT;
    static const int VALUE_TRANSFER_CONTINUOUS_ATTEMPTION_TIMES_LIMIT;

    static QString settingsAbsPath();
    static int getLogPrintServerity();
    static void saveLogPrintServerity(int serverity);
    static int getLogFileServerity();
    static void saveLogFileServerity(int serverity);
    static QString logFolderAbsPath();
    static void saveLogFolderPath(const QString &path);
    static QString pluginsAbsPath();
    static void savePluginPath(const QString &path);
    static QString defaultClassroomFileAbsDir();
    static QString ignoreGroupVersion();
    static void saveIgnoreGroupVersion(const QString &version);
    static QString upgradeUrl();
    static QString upgradeFolderAbsPath();
    static QStringList openedClassroomsRecord();
    static void saveOpenedClassroomsRecord(const QStringList &openedClassroomsPathes);
    static QString sharedFolderAbsPath();
    static QString deviceUpgradeFolderAbsPath(const QString &deviceType);

    static int upgradeTimeLimit();
    static int transferAttemptionTimesLimit();
    static int parallelTransferDeviceCountLimit();
    static int transferContinuousAttemptionTimesLimit();
};

#endif // SETTINGS_H
