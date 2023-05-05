#ifndef CONFIG_H
#define CONFIG_H

#include "utilsGlobal.h"

#include <QString>

class UTILS_EXPORT Config
{
public:
    // folder
    static const QString CONFIG_FOLDER;
    // file
    static const QString CONFIG_FILE;
    static const QString INIT_FLAG_FILE;
    static const QString XML_DB_FILE;
    // section
    static const QString LOG_SECTION;
    static const QString ENV_SECTION;
    static const QString LOGIN_SECTION;
    static const QString BUFFER_SECTION;
    static const QString WEB_CLIENT_SECTION;
    static const QString MODULE_SECTION;
    static const QString APP_SECTION;

    // key
    static const QString LOG_LEVEL_KEY;
    static const QString LOG_BUF_SECS_KEY;
    static const QString LOG_MAX_SIZE_KEY;
    static const QString LOG_VERBOSE_KEY;
    static const QString KEEP_TEMP_FILE_KEY;
    static const QString TARGET_ENV_KEY;
    static const QString MAX_TOUCH_POINTS_KEY;
    static const QString USER_ID_KEY;
    static const QString TOKEN_KEY;
    static const QString LOGIN_ID_KEY;
    static const QString IS_BEGIN_CLASS_KEY;
    static const QString CUR_CLASS_ID_KEY;
    static const QString IS_BEGIN_CLASS_WORK_KEY;
    static const QString WORK_CLASS_ID_KEY;
    static const QString AUTO_LOGIN_FLAG_KEY;
    static const QString UPLOAD_ORIGIN_KEY;
    static const QString EXERCISE_TIMES_KEY;
    static const QString BASE_PATH_KEY;
    static const QString TEST_VERSION_KEY;
    static const QString MIN_FREE_SIZE_KEY;
    static const QString TEST_XML_TIMESTAMP;
    static const QString TEST_DEVICE_ID;
    static const QString TEST_RESTORE_MODE;
    static const QString TEST_WRITE_SPEED;
    static const QString CHAR_EVALUATION_UPDATE_PACKAGE;
    static const QString HAND_WRITTEN_UPDATE_PACKAGE;
    static const QString AIPEN_SERVER_UPDATE_PACKAGE;
    static const QString CHAR_EVALUATION_PACKAGE_VERSION;
    static const QString HAND_WRITTEN_PACKAGE_VERSION;
    static const QString AIPEN_SERVER_PACKAGE_VERSION;
    static const QString CHAR_EVALUATION_DOWNLOADING_PACKAGE;
    static const QString HAND_WRITTEN_DOWNLOADING_PACKAGE;
    static const QString AIPEN_SERVER_DOWNLOADING_PACKAGE;
    static const QString HEART_BEAT_KEY;
    static const QString APP_CRASH_KEY;
    static const QString APP_CRASH_TEST_KEY;
    static const QString APP_RUNNING_KEY;
    static const QString APP_PORT_KEY;

    // value
    static const int LOG_LEVEL_DEFAULT_VALUE;
    static const int LOG_BUF_SECS_DEFAULT_VALUE;
    static const int LOG_MAX_SIZE_DEFAULT_VALUE;

    static const QString AIPEN_PC_PATH;
    static const QString TEMP_PATH;
    static const QString LOG_PATH;
    static const QString UPGRADE_PATH;
    static const QString XML_UPGRADE_PATH;
    static const QString CHAR_EVALUATION_UPGRADE_PATH;
    static const QString HAND_WRITTEN_UPGRADE_PATH;
    static const QString AIPEN_SERVER_UPGRADE_PATH;


    static QString configFileAbsPath();
    static int getLogLevel();
    static int getLogBufSecs();
    static int getLogMaxSize();
    static int getLogVerbose();
    static int getTargetEnv();
    static int getMaxTouchPoints();
    static int getKeepTempFile();
    static int getUploadOrigin();

    static void clearBufferIp();
    static void saveBufferIp(QString mac, QString ip);
    static void saveBufferIp(QMap<QString,QString> macIps);
    static QString getBufferIp(QString mac);
    static QStringList getMacList();

    static void saveClassInfo(const bool& bIsBeginClass, const int& nCurClassId);
    static void getClassInfo(bool& bIsBeginClass, int& nCurClassId);

    static void saveClassWorkInfo(const bool& bIsBeginClassWork, const int& nWorkClassId);
    static void getClassWorkInfo(bool& bIsBeginClassWork, int& nWorkClassId);

    static void saveLoginInfo(qint64 userId, const QString& token, const QString& loginId, const bool bAutoLogin);
    static void getLoginInfo(qint64& userId, QString& token, QString& loginId, bool &bAutoLogin);

    static void removeLoginInfo();
    static QString getLoginId();
    static void saveToken(QString token);

    static void saveExerciseTimes(QString times);
    static QString getExerciseTimes();

    static QString getAipenPath();
    static QString getBasePath();
    static QString getTempPath();
    static QString getLogPath();
    static QString getUpgradePath();
    static QString getXmlUpgradePath();
    static QString getCharEvaluationUpgradePath();
    static QString getHandWrittenUpgradePath();
    static QString getAipenServerUpgradePath();
    static QString getLastVersionPath();
    static QString getAipenConfigPath();

    static QString getTestVersion();
    static int getMinFreeSize();
    static QString getInitFlagPath();
    static QString getXmlDbPath();

    static QString localVersion();
    static QString getXmlTimestamp();
    static QString getDeviceId();
    static bool testRestoreMode();
    static int testWriteSpeed();

    static QString charEvaluationUpdatePackage(QString& version);
    static QString handWrittenUpdatePackage(QString& version);
    static QString aipenServerUpdatePackage(QString& version);
    static void saveCharEvaluationUpdatePackage(QString packagePath,QString version);
    static void saveHandWrittenUpdatePackage(QString packagePath,QString version);
    static void saveAipenServerUpdatePackage(QString packagePath,QString version);
    static void removeCharEvaluationUpdatePackage();
    static void removeHandWrittenUpdatePackage();
    static void removeAipenServerUpdatePackage();

    static QString charEvaluationDownloadingPackage();
    static QString handWrittenDownloadingPackage();
    static QString aipenServerDownloadingPackage();
    static void saveCharEvaluationDownloadingPackage(QString packagePath);
    static void saveHandWrittenDownloadingPackage(QString packagePath);
    static void saveAipenServerDownloadingPackage(QString packagePath);
    static void removeCharEvaluationDownloadingPackage();
    static void removeHandWrittenDownloadingPackage();
    static void removeAipenServerDownloadingPackage();

    static bool GetHeartbeat();
    static void SetHeartbeat(bool bHeartBeat);

    static bool GetAppCrashState();
    static void SetAppCrashState(bool bCrash);

    static void getCrashTestState(bool& bIsCrashTest);
    static bool getAppRunning();
    static void setAppRunning(bool running);

    static void savePort(const int& nPort);
    static void getPort(int& nPort);

};

#endif // CONFIG_H
