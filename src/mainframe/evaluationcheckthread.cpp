#include "evaluationcheckthread.h"

#include <QProcess>

#include "logHelper.h"
#include "config.h"
#include "httpconfig.h"
#include "projectconst.h"

const QString kProcName("wordevaluation");
const QString kAipenProcName("aipenserver");
const QString kElectronMainName("electronmain.exe");

EvaluationCheckThread::EvaluationCheckThread(QObject* parent)
    : QThread(parent)
    , running_(true)
    , m_bMainProCrash(false)
{
}

void EvaluationCheckThread::setMainProCrash(bool bMainProCrash)
{
    m_bMainProCrash = bMainProCrash;
    if(bMainProCrash) {
        electronMainFirstStart_ = false; //主程序崩溃的，那么electron不是首次启动
    }
}

bool EvaluationCheckThread::checkProc(bool emptyCheck)
{
    QProcess process;
    process.start("tasklist",QStringList() << "/NH" <<
                  "/FI" << "IMAGENAME eq wordevaluation.exe");
    process.waitForFinished();

    QByteArray result = process.readAllStandardOutput();
    QString str = result;
    if(str.contains(kProcName)) {
        return true;
    }
    LOG(INFO) << "EvaluationCheckThread::checkProc:" << result.data()
              <<",size=" << result.size() << ",code=" << process.exitCode();
    if(emptyCheck && (str.isEmpty() || 0 != process.exitCode())) {
        LOG(WARNING) << "EvaluationCheckThread::checkProc tasklist error";
        return true; //异常时结果丢弃
    }
    return false;
}

bool EvaluationCheckThread::checkAipenServerProc(bool emptyCheck)
{
    QProcess process;
    process.start("tasklist",QStringList() << "/NH" <<
                  "/FI" << "IMAGENAME eq aipenserver.exe");
    process.waitForFinished();

    QByteArray result = process.readAllStandardOutput();
    QString str = result;
    if(str.contains(kAipenProcName)) {
        return true;
    }
    LOG(INFO) << "EvaluationCheckThread::checkAipenServerProc:" << result.data()
                 <<",size=" << result.size() << ",code=" << process.exitCode();
    if(emptyCheck && (str.isEmpty() || 0 != process.exitCode())) {
        LOG(WARNING) << "EvaluationCheckThread::checkAipenServerProc tasklist error";
        return true; //异常时结果丢弃
    }
    return false;
}

bool EvaluationCheckThread::checkElectronMainProc(bool emptyCheck)
{
    QProcess process;
    process.start("tasklist",QStringList() << "/NH" <<
                  "/FI" << QString("IMAGENAME eq %1").arg(kElectronMainName));
    process.waitForFinished();

    QByteArray result = process.readAllStandardOutput();
    QString str = result;
    if(str.contains(kElectronMainName)) {
        return true;
    }
    LOG(INFO) << "EvaluationCheckThread::checkElectronMainProc:" << result.data()
                 <<",size=" << result.size() << ",code=" << process.exitCode();
    if(emptyCheck && (str.isEmpty() || 0 != process.exitCode())) {
        LOG(WARNING) << "EvaluationCheckThread::checkElectronMainProc tasklist error";
        return true; //异常时结果丢弃
    }
    return false;
}


void EvaluationCheckThread::closeProc()
{
    LOG(INFO) << "EvaluationCheckThread::closeProc";
    QProcess p;
    QString c = "taskkill /im wordevaluation.exe /f";
    p.execute(c);
    p.close();
}

void EvaluationCheckThread::closeAipenProc()
{
    LOG(INFO) << "EvaluationCheckThread::closeAipenProc";
    QProcess p;
    QString c = "taskkill /im aipenserver.exe /f";
    p.execute(c);
    p.close();
}

void EvaluationCheckThread::closeElectronProc()
{
    LOG(INFO) << "EvaluationCheckThread::closeElectronProc";
    QProcess p;
    QString c = "taskkill /im electronmain.exe /f";
    p.execute(c);
    p.close();
}

void EvaluationCheckThread::run()
{
    if (!m_bMainProCrash)
    {
        closeProc();
        closeAipenProc();
        closeElectronProc();
    }
    while(running_.load()) {
        if(!checkProc(true)) {
            LOG(INFO) << "EvaluationCheckThread::run wordevaluation need start proc";
            QProcess::startDetached(kProcName);
        }
        //增加aipenserver的守护
        if(!checkAipenServerProc(true)) {
            LOG(INFO) << "EvaluationCheckThread::run aipenserver need start proc";
            QProcess::startDetached(kAipenProcName);
        }

        //增加electronmain的守护
        if(!checkElectronMainProc(true)) {
            LOG(INFO) << "EvaluationCheckThread::run electronmain need start proc";

            bool bAutoLogin = false;
            qint64 userId = 0;
            QString token;
            QString loginId;
            Config::getLoginInfo(userId,token,loginId, bAutoLogin);
            if(!electronMainFirstStart_) {
                bAutoLogin = true;
            }
            QStringList args;
            args << QString("--target_env=%1").arg(Config::getTargetEnv());
            if(userId != 0 && !token.isEmpty() && bAutoLogin)
            {
                args << QString("--user_id=%1").arg(userId);
                args << QString("--token=%1").arg(token);
            }
            if(!loginId.isEmpty()) {
                args << QString("--login_id=%1").arg(loginId);
            }
            if(electronMainFirstStart_) {
                args << QString("--show_loading=true");
            } else {
                args << QString("--show_loading=false");
            }

            if(Config::testRestoreMode()) {
                args << QString("--restore_mode=true");
            } else {
                args << QString("--restore_mode=false");
            }

            for(QString arg : args) {
                LOG(INFO) << "arg:" << arg.toStdString();
            }

            QProcess::startDetached("electron/electronmain.exe",  args,".");
            if(electronMainFirstStart_) {
                electronMainFirstStart_ = false;
            }
        }
        sleep(1);
    }
    LOG(INFO) << "EvaluationCheckThread::run while quit";
    //退出时关闭进程
    closeProc();
    //关闭
    closeAipenProc();
    //electron由关闭按钮关闭，不需要这里自动结束
    closeElectronProc();
}
