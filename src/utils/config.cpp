#include "config.h"
#include "logHelper.h"
#include "fileDirHandler.h"

#include <QSettings>
#include <QDir>
#include <QStringList>
#include <QByteArray>
#include <QStandardPaths>
#include <QDebug>

#include "projectconst.h"
#include "commonMacro.h"
#include "httpconfig.h"

const QString Config::CONFIG_FOLDER = "configs";
const QString Config::CONFIG_FILE = "config.ini";

const QString Config::LOG_SECTION = "log";
const QString Config::BUFFER_SECTION = "mac";
const QString Config::ENV_SECTION = "env";
const QString Config::LOGIN_SECTION="login";
const QString Config::WEB_CLIENT_SECTION="web";
const QString Config::MODULE_SECTION="module";
const QString Config::APP_SECTION = "app";

const QString Config::LOG_LEVEL_KEY = "log_level";
const QString Config::LOG_BUF_SECS_KEY = "log_buf_secs";
const QString Config::LOG_MAX_SIZE_KEY = "log_max_size"; //MB
const QString Config::LOG_VERBOSE_KEY = "log_verbose";
const QString Config::KEEP_TEMP_FILE_KEY = "keep_temp_file"; //0关闭，1打开
const QString Config::TARGET_ENV_KEY = "target_env";
const QString Config::MAX_TOUCH_POINTS_KEY = "max_touch_points";
const QString Config::USER_ID_KEY = "user_id";
const QString Config::TOKEN_KEY = "token";
const QString Config::LOGIN_ID_KEY = "login_id";
const QString Config::AUTO_LOGIN_FLAG_KEY = "auto_login";
const QString Config::IS_BEGIN_CLASS_KEY = "isBeginClass";
const QString Config::CUR_CLASS_ID_KEY = "curClassId";
const QString Config::IS_BEGIN_CLASS_WORK_KEY = "isBeginClassWork";
const QString Config::WORK_CLASS_ID_KEY = "workClassId";

const QString Config::UPLOAD_ORIGIN_KEY = "upload_origin";
const QString Config::EXERCISE_TIMES_KEY = "exercise_times";
const QString Config::BASE_PATH_KEY = "base_path";
const QString Config::TEST_VERSION_KEY = "test_version";
const QString Config::MIN_FREE_SIZE_KEY = "min_free_size"; //MB
const QString Config::TEST_XML_TIMESTAMP = "xml_timestamp";
const QString Config::TEST_DEVICE_ID = "device_id";
const QString Config::TEST_RESTORE_MODE = "restore_mode";
const QString Config::TEST_WRITE_SPEED = "write_speed";
const QString Config::CHAR_EVALUATION_UPDATE_PACKAGE = "char_evaluation_update_package";
const QString Config::HAND_WRITTEN_UPDATE_PACKAGE = "hand_written_update_package";
const QString Config::AIPEN_SERVER_UPDATE_PACKAGE = "aipen_server_update_package";
const QString Config::CHAR_EVALUATION_PACKAGE_VERSION = "char_evaluation_package_version";
const QString Config::HAND_WRITTEN_PACKAGE_VERSION = "hand_written_package_version";
const QString Config::AIPEN_SERVER_PACKAGE_VERSION = "aipen_server_package_version";
const QString Config::CHAR_EVALUATION_DOWNLOADING_PACKAGE = "char_evaluation_downloading_package";
const QString Config::HAND_WRITTEN_DOWNLOADING_PACKAGE = "hand_written_downloading_package";
const QString Config::AIPEN_SERVER_DOWNLOADING_PACKAGE = "aipen_server_downloading_package";
const QString Config::HEART_BEAT_KEY = "heartBeat";
const QString Config::APP_CRASH_KEY = "appCrash";
const QString Config::APP_CRASH_TEST_KEY = "appCrashTest";
const QString Config::APP_RUNNING_KEY = "app_running";
const QString Config::APP_PORT_KEY = "port";

const int Config::LOG_LEVEL_DEFAULT_VALUE = 1; // 0 - info; 1 - warning; 2 - error; 3 - fatal
const int Config::LOG_BUF_SECS_DEFAULT_VALUE = 0; //立即输出
const int Config::LOG_MAX_SIZE_DEFAULT_VALUE = 50;


const QString Config::AIPEN_PC_PATH = "aipen_pc/";
const QString Config::TEMP_PATH = "temp/";
const QString Config::LOG_PATH = "log/";
const QString Config::UPGRADE_PATH = "upgrade/";
const QString Config::XML_UPGRADE_PATH = "xml_upgrade/";
const QString Config::CHAR_EVALUATION_UPGRADE_PATH = "char_evaluation_upgrade/";
const QString Config::HAND_WRITTEN_UPGRADE_PATH = "hand_written_upgrade/";
const QString Config::AIPEN_SERVER_UPGRADE_PATH = "aipen_server_upgrade/";
const QString Config::INIT_FLAG_FILE = "initFlag";
const QString Config::XML_DB_FILE = "xml_timestamp.db";

const QString kMainVersion { MARCO2STR(STR_MAIN_VERSION) };
const QString kCharEvaluationVersion { MARCO2STR(STR_CHAREVALUATION_VERSION) };
const QString kHandWrittenVersion { MARCO2STR(STR_HANDWRITTEN_VERSION) };

QString Config::configFileAbsPath()
{
    QString configFilePath = CONFIG_FOLDER + "/" + CONFIG_FILE;
    configFilePath = FileDirHandler::absolutePath(configFilePath);
    return configFilePath;
}

int Config::getLogLevel()
{
    QSettings settings(configFileAbsPath(), QSettings::IniFormat);
    settings.beginGroup(LOG_SECTION);
    int level = settings.value(LOG_LEVEL_KEY, LOG_LEVEL_DEFAULT_VALUE).toInt();
    settings.endGroup();
    return level;
}

int Config::getLogBufSecs()
{
    QSettings settings(configFileAbsPath(), QSettings::IniFormat);
    settings.beginGroup(LOG_SECTION);
    int bufSecs = settings.value(LOG_BUF_SECS_KEY, LOG_BUF_SECS_DEFAULT_VALUE).toInt();
    settings.endGroup();
    return bufSecs;
}

int Config::getLogMaxSize()
{
    QSettings settings(configFileAbsPath(), QSettings::IniFormat);
    settings.beginGroup(LOG_SECTION);
    int maxSize = settings.value(LOG_MAX_SIZE_KEY, LOG_MAX_SIZE_DEFAULT_VALUE).toInt();
    settings.endGroup();
    return maxSize;
}

int Config::getLogVerbose()
{
    QSettings settings(configFileAbsPath(), QSettings::IniFormat);
    settings.beginGroup(LOG_SECTION);
    int verbose = settings.value(LOG_VERBOSE_KEY, 0).toInt();
    settings.endGroup();
    return verbose;
}

int Config::getTargetEnv()
{
    QSettings settings(configFileAbsPath(), QSettings::IniFormat);
    settings.beginGroup(ENV_SECTION);
    int targetEnv = settings.value(TARGET_ENV_KEY, static_cast<int>(Env::release)).toInt();
    settings.endGroup();
    return targetEnv;
}

int Config::getMaxTouchPoints()
{
    QSettings settings(configFileAbsPath(), QSettings::IniFormat);
    settings.beginGroup(ENV_SECTION);
    int maxTouchPoints = settings.value(MAX_TOUCH_POINTS_KEY, 1).toInt();
    settings.endGroup();
    return maxTouchPoints;
}

int Config::getKeepTempFile()
{
    QSettings settings(configFileAbsPath(), QSettings::IniFormat);
    settings.beginGroup(LOG_SECTION);
    int targetEnv = settings.value(KEEP_TEMP_FILE_KEY, 0).toInt();
    settings.endGroup();
    return targetEnv;
}

int Config::getUploadOrigin()
{
    QSettings settings(configFileAbsPath(), QSettings::IniFormat);
    settings.beginGroup(LOG_SECTION);
    int targetEnv = settings.value(UPLOAD_ORIGIN_KEY, 0).toInt();
    settings.endGroup();
    return targetEnv;
}

void Config::clearBufferIp()
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(BUFFER_SECTION);
    QStringList macs = settings.allKeys();
    for(auto mac : macs) {
        settings.remove(mac);
    }
    settings.endGroup();
}

void Config::saveBufferIp(QString mac, QString ip)
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(BUFFER_SECTION);
    settings.setValue(mac,ip);
    settings.endGroup();
}

void Config::saveBufferIp(QMap<QString,QString> macIps)
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(BUFFER_SECTION);
    QStringList macs = settings.allKeys();
    for(auto mac : macs) {
        if(!macIps.contains(mac)) {
            settings.remove(mac);
        }
    }
    for(auto cit = macIps.cbegin(); cit != macIps.cend(); ++cit) {
        settings.setValue(cit.key(),cit.value());
    }
    settings.endGroup();
}

QString Config::getBufferIp(QString mac)
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(BUFFER_SECTION);
    QString ip = settings.value(mac).toString();
    settings.endGroup();
    return ip;
}

QStringList Config::getMacList()
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(BUFFER_SECTION);
    QStringList macList = settings.allKeys();
    settings.endGroup();
    return macList;
}

void Config::saveClassInfo(const bool &bIsBeginClass, const int &nCurClassId)
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(LOGIN_SECTION);
    settings.setValue(IS_BEGIN_CLASS_KEY, bIsBeginClass);
    if (nCurClassId != 0)
    {
        settings.setValue(CUR_CLASS_ID_KEY,nCurClassId);
    }
    else
    {
        settings.remove(CUR_CLASS_ID_KEY);
    }

    settings.endGroup();
}

void Config::getClassInfo(bool &bIsBeginClass, int &nCurClassId)
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(LOGIN_SECTION);
    bIsBeginClass = settings.value(IS_BEGIN_CLASS_KEY,0).toBool();
    nCurClassId = settings.value(CUR_CLASS_ID_KEY, 0).toInt();
    settings.endGroup();
}



void Config::saveClassWorkInfo(const bool &bIsBeginClassWork, const int &nWorkClassId)
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(LOGIN_SECTION);
    settings.setValue(IS_BEGIN_CLASS_WORK_KEY, bIsBeginClassWork);
    if (nWorkClassId != 0)
    {
        settings.setValue(WORK_CLASS_ID_KEY,nWorkClassId);
    }
    else
    {
        settings.remove(WORK_CLASS_ID_KEY);
    }
    settings.endGroup();
}

void Config::getClassWorkInfo(bool &bIsBeginClassWork, int &nWorkClassId)
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(LOGIN_SECTION);
    bIsBeginClassWork = settings.value(IS_BEGIN_CLASS_WORK_KEY,0).toBool();
    nWorkClassId = settings.value(WORK_CLASS_ID_KEY, 0).toInt();
    settings.endGroup();
}

void Config::saveLoginInfo(qint64 userId, const QString& token, const QString& loginId, const bool bAutoLogin)
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(LOGIN_SECTION);
    if(userId != 0) {
        settings.setValue(USER_ID_KEY,userId);
    } else {
        settings.remove(USER_ID_KEY);
    }
    if(!token.isEmpty()) {
        settings.setValue(TOKEN_KEY,token);
    } else {
        settings.remove(TOKEN_KEY);
    }
    settings.setValue(AUTO_LOGIN_FLAG_KEY, bAutoLogin);
    settings.setValue(LOGIN_ID_KEY,loginId);
    settings.endGroup();
}

void Config::getLoginInfo(qint64& userId, QString& token, QString& loginId, bool& bAutoLogin)
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(LOGIN_SECTION);
    userId = settings.value(USER_ID_KEY,0).toLongLong();
    token = settings.value(TOKEN_KEY,"").toString();
    loginId = settings.value(LOGIN_ID_KEY,"").toString();
    bAutoLogin = settings.value(AUTO_LOGIN_FLAG_KEY, 0).toBool();
    settings.endGroup();
}

void Config::removeLoginInfo()
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(LOGIN_SECTION);
    settings.remove(USER_ID_KEY);
    settings.remove(TOKEN_KEY);
    settings.endGroup();
}

QString Config::getLoginId()
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(LOGIN_SECTION);
    QString loginId = settings.value(LOGIN_ID_KEY,"").toString();
    settings.endGroup();
    return loginId;
}

void Config::saveExerciseTimes(QString times)
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(WEB_CLIENT_SECTION);
    settings.setValue(EXERCISE_TIMES_KEY, times);
    settings.endGroup();
}

QString Config::getExerciseTimes()
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(WEB_CLIENT_SECTION);
    QString exerciseTimes = settings.value(EXERCISE_TIMES_KEY, "").toString();
    settings.endGroup();
    return exerciseTimes;
}

QString Config::getAipenPath()
{
    QString basePath = getBasePath();
    return (basePath + AIPEN_PC_PATH);
}

QString Config::getBasePath()
{
    QSettings settings(configFileAbsPath(), QSettings::IniFormat);
    settings.beginGroup(ENV_SECTION);
    QString basePath = settings.value(BASE_PATH_KEY, "").toString();
    settings.endGroup();
    if(basePath.isEmpty()) {
        basePath = ".."; //避免放到C盘documents导致被还原掉 QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }
    if(!basePath.endsWith("/")) {
        basePath += "/";
    }
    return basePath;
}

QString Config::getTempPath()
{
    QString aipenPath = getAipenPath();
    return (aipenPath + TEMP_PATH);
}

QString Config::getLogPath()
{
    QString aipenPath = getAipenPath();
    return (aipenPath + LOG_PATH);
}

QString Config::getUpgradePath()
{
    QString aipenPath = getAipenPath();
    return (aipenPath + UPGRADE_PATH);
}

QString Config::getXmlUpgradePath()
{
    QString aipenPath = getAipenPath();
    return (aipenPath + XML_UPGRADE_PATH);
}

QString Config::getCharEvaluationUpgradePath()
{
    QString aipenPath = getAipenPath();
    return (aipenPath + CHAR_EVALUATION_UPGRADE_PATH);
}

QString Config::getHandWrittenUpgradePath()
{
    QString aipenPath = getAipenPath();
    return (aipenPath + HAND_WRITTEN_UPGRADE_PATH);
}

QString Config::getAipenServerUpgradePath()
{
    QString aipenPath = getAipenPath();
    return (aipenPath + AIPEN_SERVER_UPGRADE_PATH);
}

QString Config::getAipenConfigPath()
{
    QString aipenPath = getAipenPath();
    return (aipenPath + "aipen_config.ini");
}

QString Config::getTestVersion()
{
    QSettings settings(configFileAbsPath(), QSettings::IniFormat);
    settings.beginGroup(ENV_SECTION);
    QString testVer = settings.value(TEST_VERSION_KEY, "").toString();
    settings.endGroup();
    return testVer;
}

int Config::getMinFreeSize()
{
    QSettings settings(configFileAbsPath(), QSettings::IniFormat);
    settings.beginGroup(ENV_SECTION);
    int minFreeSize = settings.value(MIN_FREE_SIZE_KEY, kMinFreeSize).toInt();
    settings.endGroup();
    return minFreeSize;
}

QString Config::getInitFlagPath()
{
    QString configFilePath = CONFIG_FOLDER + "/" + INIT_FLAG_FILE;
    configFilePath = FileDirHandler::absolutePath(configFilePath);
    return configFilePath;
}

QString Config::getXmlDbPath()
{
    QString aipenPath = getAipenPath();
    return (aipenPath + XML_DB_FILE);
}

QString Config::localVersion()
{
    QString localVersion = getTestVersion();
    if(localVersion.isEmpty()) {
        localVersion = kMainVersion;
    }
    return localVersion;
}

QString Config::getXmlTimestamp()
{
    QSettings settings(configFileAbsPath(), QSettings::IniFormat);
    settings.beginGroup(ENV_SECTION);
    QString xmlTimestamp = settings.value(TEST_XML_TIMESTAMP, kXMLUpdateTime).toString();
    settings.endGroup();
    return xmlTimestamp;
}

QString Config::getDeviceId()
{
    QSettings settings(configFileAbsPath(), QSettings::IniFormat);
    settings.beginGroup(ENV_SECTION);
    QString deviceId = settings.value(TEST_DEVICE_ID, kDeviceId).toString();
    settings.endGroup();
    return deviceId;
}

bool Config::testRestoreMode()
{
    QSettings settings(configFileAbsPath(), QSettings::IniFormat);
    settings.beginGroup(ENV_SECTION);
    bool bRestoreMode = settings.value(TEST_RESTORE_MODE, false).toBool();
    settings.endGroup();
    return bRestoreMode;
}

int Config::testWriteSpeed()
{
    QSettings settings(configFileAbsPath(), QSettings::IniFormat);
    settings.beginGroup(ENV_SECTION);
    int writeSpeed = settings.value(TEST_WRITE_SPEED, 0).toInt();
    settings.endGroup();
    return writeSpeed;
}

bool Config::GetHeartbeat()
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(LOGIN_SECTION);
    QString strHeartBeat = settings.value(HEART_BEAT_KEY,"").toString();
    bool bHeartBeat = false;
    if (strHeartBeat.compare("true") == 0)
    {
        bHeartBeat = true; //没有配置也被认为false
    }
    return bHeartBeat;
}

void Config::SetHeartbeat(bool bHeartBeat)
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(LOGIN_SECTION);
    settings.setValue(HEART_BEAT_KEY,bHeartBeat?"true":"false");
    settings.endGroup();
}

bool Config::GetAppCrashState()
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(APP_SECTION);
    bool bIsCrash = false;
    bIsCrash = settings.value(APP_CRASH_KEY, 0).toBool();
    return bIsCrash;
}

void Config::SetAppCrashState(bool bCrash)
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(APP_SECTION);
    settings.setValue(APP_CRASH_KEY, bCrash);
    settings.endGroup();
}

void Config::getCrashTestState(bool &bIsCrashTest)
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(APP_SECTION);
    bIsCrashTest = settings.value(APP_CRASH_TEST_KEY,0).toBool();
    settings.endGroup();
}

bool Config::getAppRunning()
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(APP_SECTION);
    bool appRunning = settings.value(APP_RUNNING_KEY,false).toBool();
    settings.endGroup();
    return appRunning;
}

void Config::setAppRunning(bool running)
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(APP_SECTION);
    settings.setValue(APP_RUNNING_KEY,running);
    settings.endGroup();
}

QString Config::charEvaluationUpdatePackage(QString& version)
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(MODULE_SECTION);
    QString testVer = settings.value(CHAR_EVALUATION_UPDATE_PACKAGE, "").toString();
    version = settings.value(CHAR_EVALUATION_PACKAGE_VERSION, "").toString();
    settings.endGroup();
    return testVer;
}

QString Config::handWrittenUpdatePackage(QString& version)
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(MODULE_SECTION);
    QString testVer = settings.value(HAND_WRITTEN_UPDATE_PACKAGE, "").toString();
    version = settings.value(HAND_WRITTEN_PACKAGE_VERSION, "").toString();
    settings.endGroup();
    return testVer;
}

QString Config::aipenServerUpdatePackage(QString& version)
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(MODULE_SECTION);
    QString testVer = settings.value(AIPEN_SERVER_UPDATE_PACKAGE, "").toString();
    version = settings.value(AIPEN_SERVER_PACKAGE_VERSION, "").toString();
    settings.endGroup();
    return testVer;
}

void Config::saveCharEvaluationUpdatePackage(QString packagePath,QString version)
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(MODULE_SECTION);
    settings.setValue(CHAR_EVALUATION_UPDATE_PACKAGE, packagePath);
    settings.setValue(CHAR_EVALUATION_PACKAGE_VERSION, version);
    settings.endGroup();
}


void Config::saveHandWrittenUpdatePackage(QString packagePath,QString version)
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(MODULE_SECTION);
    settings.setValue(HAND_WRITTEN_UPDATE_PACKAGE, packagePath);
    settings.setValue(HAND_WRITTEN_PACKAGE_VERSION, version);
    settings.endGroup();
}

void Config::saveAipenServerUpdatePackage(QString packagePath,QString version)
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(MODULE_SECTION);
    settings.setValue(AIPEN_SERVER_UPDATE_PACKAGE, packagePath);
    settings.setValue(AIPEN_SERVER_PACKAGE_VERSION, version);
    settings.endGroup();
}

void Config::removeCharEvaluationUpdatePackage()
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(MODULE_SECTION);
    settings.remove(CHAR_EVALUATION_UPDATE_PACKAGE);
    settings.remove(CHAR_EVALUATION_PACKAGE_VERSION);
    settings.endGroup();
}

void Config::removeHandWrittenUpdatePackage()
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(MODULE_SECTION);
    settings.remove(HAND_WRITTEN_UPDATE_PACKAGE);
    settings.remove(HAND_WRITTEN_PACKAGE_VERSION);
    settings.endGroup();
}

void Config::removeAipenServerUpdatePackage()
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(MODULE_SECTION);
    settings.remove(AIPEN_SERVER_UPDATE_PACKAGE);
    settings.remove(AIPEN_SERVER_PACKAGE_VERSION);
    settings.endGroup();
}

QString Config::charEvaluationDownloadingPackage()
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(MODULE_SECTION);
    QString testVer = settings.value(CHAR_EVALUATION_DOWNLOADING_PACKAGE, "").toString();
    settings.endGroup();
    return testVer;
}

QString Config::handWrittenDownloadingPackage()
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(MODULE_SECTION);
    QString testVer = settings.value(HAND_WRITTEN_DOWNLOADING_PACKAGE, "").toString();
    settings.endGroup();
    return testVer;
}

QString Config::aipenServerDownloadingPackage()
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(MODULE_SECTION);
    QString testVer = settings.value(AIPEN_SERVER_DOWNLOADING_PACKAGE, "").toString();
    settings.endGroup();
    return testVer;
}

void Config::saveCharEvaluationDownloadingPackage(QString packagePath)
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(MODULE_SECTION);
    settings.setValue(CHAR_EVALUATION_DOWNLOADING_PACKAGE, packagePath);
    settings.endGroup();
}

void Config::saveHandWrittenDownloadingPackage(QString packagePath)
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(MODULE_SECTION);
    settings.setValue(HAND_WRITTEN_DOWNLOADING_PACKAGE, packagePath);
    settings.endGroup();
}

void Config::saveAipenServerDownloadingPackage(QString packagePath)
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(MODULE_SECTION);
    settings.setValue(AIPEN_SERVER_DOWNLOADING_PACKAGE, packagePath);
    settings.endGroup();
}

void Config::removeCharEvaluationDownloadingPackage()
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(MODULE_SECTION);
    settings.remove(CHAR_EVALUATION_DOWNLOADING_PACKAGE);
    settings.endGroup();
}

void Config::removeHandWrittenDownloadingPackage()
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(MODULE_SECTION);
    settings.remove(HAND_WRITTEN_DOWNLOADING_PACKAGE);
    settings.endGroup();
}

void Config::removeAipenServerDownloadingPackage()
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(MODULE_SECTION);
    settings.remove(AIPEN_SERVER_DOWNLOADING_PACKAGE);
    settings.endGroup();
}

void Config::saveToken(QString token)
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(LOGIN_SECTION);
    settings.setValue(TOKEN_KEY,token);
    settings.endGroup();
}

void Config::savePort(const int &nPort)
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(APP_SECTION);
    settings.setValue(APP_PORT_KEY, nPort);
    settings.endGroup();
}

void Config::getPort(int &nPort)
{
    QString qstrfilename = getAipenConfigPath();
    QSettings settings(qstrfilename,QSettings::IniFormat);
    settings.beginGroup(APP_SECTION);
    nPort = settings.value(APP_PORT_KEY,0).toInt();
    settings.endGroup();
}
