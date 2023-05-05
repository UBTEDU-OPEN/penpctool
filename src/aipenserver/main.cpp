#include <QCoreApplication>
#include <QDir>
#include <QGuiApplication>
#include "aipenserver.h"
#include "logHelper.h"
#include "config.h"
#include "localxmlworker.h"
#include <ctime>
#include <QTime>
#include <QProcess>
#include <QDir>
#include <QRandomGenerator>
#include <QJsonDocument>
#include <QJsonObject>

#ifdef _WIN32
#include <Windows.h>
#include "exceptionhandler.h"
#endif


#ifdef _WIN32
LONG WINAPI ExpFileter(PEXCEPTION_POINTERS pExp)
{
    std::string stackInfo = StackTracer::GetExceptionStackTrace(pExp);
    LOG(WARNING) << "ExpFileter occurred=================";
    LOG(WARNING) << stackInfo;
    LOG(WARNING) << "ExpFileter END=================";
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

int main(int argc, char *argv[])
{
//#ifdef _WIN32
//    CR_INSTALL_INFO info;
//    int nInstResult = crInstall(&info);
//    assert(nInstResult==0);

//    nInstResult = crAddScreenshot(CR_AS_MAIN_WINDOW);
//    assert(nInstResult==0);

//    // Check result
//    if(nInstResult!=0)
//    {
//        printf("Install crash exporter failed\n");
////            _getch();
//        return 0;
//    }
//#endif
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES); //TODO win7?(初步验证可以)解决win7休眠后崩溃问题

    qRegisterMetaType<HANDLE>("HANDLE");
    qRegisterMetaType<QProcess::ProcessState>("QProcess::ProcessState");

    qDebug() << "\n" << "random:" << QRandomGenerator::global()->generate();
    uint temp = QTime(0,0,0).secsTo(QTime::currentTime());
    qDebug() << "time temp" << temp;

    qsrand(temp);

    QGuiApplication a(argc, argv);

//    initPath();
//    checkInitFlag();

#ifdef _WIN32
    SetUnhandledExceptionFilter(ExpFileter);
    LOG(INFO) << "##### win 32 os";
#endif

    LocalXmlWorker::instance()->installXml();

    LogHelper::init(argv[0]);

    QFile file("aipenserver.json");
    QString localVer;
    if(file.open(QFile::ReadOnly)) {
        auto doc = QJsonDocument::fromJson(file.readAll());
        QJsonObject obj = doc.object();
        localVer = obj.value("version").toString();
        file.close();
    }

    LOG(INFO) << "aipenserver version:" << localVer.toStdString();
    AiPenServer server;
    int ret =  a.exec();
    return ret;
}
