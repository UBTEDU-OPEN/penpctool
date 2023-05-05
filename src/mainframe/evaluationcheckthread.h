#ifndef EVALUATIONCHECKTHREAD_H
#define EVALUATIONCHECKTHREAD_H

#include <QThread>
#include <QProcess>

#include <atomic>

class EvaluationCheckThread : public QThread
{
    Q_OBJECT
public:
    explicit EvaluationCheckThread(QObject* parent = nullptr);

    void stopRunning() { running_ = false; }

    void setMainProCrash(bool bMainProCrash);
    //刚从睡眠恢复程序，tasklist返回空，这种情况需要过滤掉,没有进程的情况会提示没有运行而不是空
    static bool checkProc(bool emptyCheck);
    static bool checkAipenServerProc(bool emptyCheck);
    static bool checkElectronMainProc(bool emptryCheck);

    void run() override;

    void closeProc();
    void closeAipenProc();
    void closeElectronProc();

private:
    std::atomic<bool> running_;
    bool m_bMainProCrash; //主进程是否崩溃重启
    bool electronMainFirstStart_ = true;
};

#endif // EVALUATIONCHECKTHREAD_H
