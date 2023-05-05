#include "mainframe.h"
#include <QApplication>
#include <QtSingleApplication>

#include <ctime>
#include <QTime>
#include <QProcess>
#include <QDir>
#include <QRandomGenerator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "logHelper.h"
#include "exceptionhandler.h"
#include "projectconst.h"
#include "config.h"
#include "fileDirHandler.h"


#ifdef _WIN32
LONG WINAPI ExpFileter(PEXCEPTION_POINTERS pExp)
{
    std::string stackInfo = StackTracer::GetExceptionStackTrace(pExp);
    LOG(WARNING) << "ExpFileter occurred=================";
    LOG(WARNING) << stackInfo;
    LOG(WARNING) << "ExpFileter END=================";
    QString appFilePath = QCoreApplication::applicationFilePath();
    LOG(INFO) << "appFilePath:" <<appFilePath.toStdString();
    Config::SetAppCrashState(true);//标记崩溃状态
    QProcess::startDetached(appFilePath, QStringList()); //重启进程
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

void clearFolder(QString path)
{
    QDir dir(path);
    QStringList files = dir.entryList(QDir::Files);
    for(QString file : files) {
        dir.remove(file);
    }
}

//TODO temp文件如果删除不掉或者无法创建需要处理
void initPath()
{
    auto basePath = Config::getBasePath();
    QDir dir(basePath);
    QString targetAipenPcPath = dir.absoluteFilePath(Config::AIPEN_PC_PATH);
    if(!dir.exists(Config::AIPEN_PC_PATH)) {
        bool isDot = false;
        if(dir.exists("1.2.0.55")) { //已有学校临时方案的拷贝
            dir.cd("1.2.0.55");
            if(dir.exists(Config::AIPEN_PC_PATH)) {
                isDot = true;
                FileDirHandler::copyDir(
                            dir.absoluteFilePath(Config::AIPEN_PC_PATH),
                            targetAipenPcPath);
            }
            dir.cdUp();
        }
        if(!isDot) {
            //不存在，但是文档下存在，拷贝过来
            QDir docDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
            if(docDir.exists(Config::AIPEN_PC_PATH)) {
                FileDirHandler::copyDir(docDir.absoluteFilePath(Config::AIPEN_PC_PATH),
                                        targetAipenPcPath);
            }
        }
    }
    if(!dir.exists(Config::AIPEN_PC_PATH)) {
        dir.mkdir(Config::AIPEN_PC_PATH);
    }
    dir.cd(Config::AIPEN_PC_PATH);
    if(!dir.exists(Config::TEMP_PATH)) {
        dir.mkdir(Config::TEMP_PATH);
    } else {
//        dir.cd(Config::TEMP_PATH);
//        dir.removeRecursively();
//        dir.cdUp();
//        dir.mkdir(Config::TEMP_PATH);
        clearFolder(Config::getTempPath());
    }

    if(!dir.exists(Config::UPGRADE_PATH)) {
        dir.mkdir(Config::UPGRADE_PATH);
    }
    if(!dir.exists(Config::XML_UPGRADE_PATH)) {
        dir.mkdir(Config::XML_UPGRADE_PATH);
    }
    if(!dir.exists(Config::CHAR_EVALUATION_UPGRADE_PATH)) {
        dir.mkdir(Config::CHAR_EVALUATION_UPGRADE_PATH);
    }
    if(!dir.exists(Config::HAND_WRITTEN_UPGRADE_PATH)) {
        dir.mkdir(Config::HAND_WRITTEN_UPGRADE_PATH);
    }
    if(!dir.exists(Config::AIPEN_SERVER_UPGRADE_PATH)) {
        dir.mkdir(Config::AIPEN_SERVER_UPGRADE_PATH);
    }
    if(!dir.exists(Config::LOG_PATH)) {
        dir.mkdir(Config::LOG_PATH);
    }
}



void checkInitFlag()
{
    QString initFlagPath = Config::getInitFlagPath();
    if(QFile::exists(initFlagPath))
    {
        QString dirPath = QCoreApplication::applicationDirPath();
        QFileInfo info(dirPath);
        QString localVer = info.fileName(); //通过文件夹名称来获取，Config::localVersion可能会被修改
        QDir dir(dirPath+"/../");
        LOG(INFO) << "dir=" << dir.absolutePath().toStdString() << "," << localVer.toStdString();
        if(0 == QFileInfo(dir.absolutePath()).fileName().compare("aipen")) {
            QRegularExpression exp("^\\d+.\\d+.\\d+.\\d+$");
            QStringList list = dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot);
            for(QString tempStr : list) {
                if(tempStr != localVer && exp.match(tempStr).hasMatch()) {
                    LOG(INFO) << "remove old version:" << tempStr.toStdString();
                    QDir tempDir(dir.absoluteFilePath(tempStr));
                    tempDir.removeRecursively();
                }
            }
        }

        if(QFile::exists(Config::getXmlDbPath())) {
            QFile::remove(Config::getXmlDbPath());
        }

        clearFolder(Config::getCharEvaluationUpgradePath());
        clearFolder(Config::getHandWrittenUpgradePath());
        clearFolder(Config::getAipenServerUpgradePath());
        clearFolder(Config::getUpgradePath());
        clearFolder(Config::getXmlUpgradePath());
        Config::removeCharEvaluationDownloadingPackage();
        Config::removeCharEvaluationUpdatePackage();
        Config::removeHandWrittenDownloadingPackage();
        Config::removeHandWrittenUpdatePackage();
        Config::removeAipenServerDownloadingPackage();
        Config::removeAipenServerUpdatePackage();

        QFile::remove(initFlagPath);
    }
}

int umcompressProgram(QString packagePath)
{
    LOG(INFO) << "umcompressProgram " << packagePath.toStdString();
    LOG(INFO) << "umcompressProgram install path="
              << QCoreApplication::applicationDirPath().toStdString();
    QString extractCmd = "7z.exe";
    QStringList extractArguments;
    extractArguments.append("x");
    extractArguments.append("-o" + QCoreApplication::applicationDirPath());
    extractArguments.append(packagePath);
    extractArguments.append("-aoa");
    QProcess extractProcess;
    extractProcess.start(extractCmd, extractArguments);
    extractProcess.waitForFinished(-1);
    int exitCode = extractProcess.exitCode();
    LOG(INFO) << "umcompressProgram ret=" << exitCode;
    if(0 != exitCode) {
        LOG(WARNING) << "umcompressProgram out=" << extractProcess.readAll().toStdString();
    }
    return exitCode;
}

int compareVersion(QString oldVersion, QString newVersion)
{
    QStringList oldVers = oldVersion.split('.');
    QStringList newVers = newVersion.split('.');

    for(int i = 0; i < 4; ++i) {
        int oldVer = oldVers[i].toInt();
        int newVer = newVers[i].toInt();
        if(newVer > oldVer) {
            return 1;
        } else if(newVer < oldVer) {
            return -1;
        }
    }

    return 0;
}

QString localVersion(QString fileName)
{
    QFile file(fileName);
    QString localVer;
    if(file.open(QFile::ReadOnly)) {
        auto doc = QJsonDocument::fromJson(file.readAll());
        QJsonObject obj = doc.object();
        localVer = obj.value("version").toString();
        file.close();
    }
    return localVer;
}

void checkUpdateModules()
{
    QString professorVer = localVersion("charevaluation.json");
    QString newVersion;
    auto newPackage = Config::charEvaluationUpdatePackage(newVersion);
    LOG(INFO) << "char evaluation update package:" << newPackage.toStdString()<<",newVerson:"<<newVersion.toStdString();
    if(!newPackage.isEmpty() && !newVersion.isEmpty() && !professorVer.isEmpty()) {
        if(1 == compareVersion(professorVer,newVersion)) {
            umcompressProgram(newPackage);
        }
        bool ret = QFile::remove(newPackage);
        LOG(INFO) << "char evaluation update package remove ret:" << ret;
        Config::removeCharEvaluationUpdatePackage();
    }

    newVersion = "";
    QString ocrVer = localVersion("handwritten.json");
    newPackage = Config::handWrittenUpdatePackage(newVersion);
    LOG(INFO) << "hand Written update package:" << newPackage.toStdString()<<",newVerson:"<<newVersion.toStdString();
    if(!newPackage.isEmpty() && !newVersion.isEmpty() && !ocrVer.isEmpty()) {
        if(1 == compareVersion(ocrVer,newVersion)) {
            umcompressProgram(newPackage);
        }
        bool ret = QFile::remove(newPackage);
        LOG(INFO) << "hand Written update package remove ret:" << ret;
        Config::removeHandWrittenUpdatePackage();
    }

    newVersion = "";
    QString aipenServerVer = localVersion("aipenserver.json");
    newPackage = Config::aipenServerUpdatePackage(newVersion);
    LOG(INFO) << "aipen server update package:" << newPackage.toStdString()<<",newVerson:"<<newVersion.toStdString();
    if(!newPackage.isEmpty() && !newVersion.isEmpty() && !aipenServerVer.isEmpty()) {
        if(1 == compareVersion(aipenServerVer,newVersion)) {
            umcompressProgram(newPackage);
        }
        bool ret = QFile::remove(newPackage);
        LOG(INFO) << "aipen server update package remove ret:" << ret;
        Config::removeAipenServerUpdatePackage();
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES); //TODO win7?(初步验证可以)解决win7休眠后崩溃问题

    qRegisterMetaType<HANDLE>("HANDLE");
    qRegisterMetaType<QProcess::ProcessState>("QProcess::ProcessState");


    QtSingleApplication a("aipen", argc, argv);
    a.setApplicationName("aipen");

    //initPath需要在checkInitFlag之前
    initPath();

    LogHelper::init(argv[0]);
    LOG(INFO) << "aipen version:" << Config::localVersion().toStdString();

    for(int i = 0; i < 30; ++i) {
        if (a.isRunning())
        {
            QThread::sleep(1);
        } else {
            break;
        }
    }
    if (a.isRunning())
    {
        a.sendMessage("aipen");
        return EXIT_SUCCESS;
    }

    checkInitFlag();
    checkUpdateModules();


#ifdef _WIN32
    SetUnhandledExceptionFilter(ExpFileter);
    LOG(INFO) << "##### win 32 os";
    if (!::SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED)) {
        LOG(ERROR) << "SetThreadExecutionState failed";
    } else {
        LOG(INFO) << "SetThreadExecutionState success";
    }
#endif

    MainFrame* w = new MainFrame;
//    w->showMaximized();

    a.setActivationWindow(w,true);
    QObject::connect(&a, &QtSingleApplication::messageReceived, w, &MainFrame::handleMessage);
    a.setQuitOnLastWindowClosed(false); //避免调用点集时打开的弹窗关闭后程序退出
    int nRet = a.exec();
    LOG(INFO) << "nRet=" << nRet;
    delete w;
    return nRet;
}
