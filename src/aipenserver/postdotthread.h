#ifndef PostDotThread_H
#define PostDotThread_H

#include <QThread>
#include <QMap>
#include <QRecursiveMutex>
#include <QFile>

class PostDotThread : public QThread
{
    Q_OBJECT
public:
    explicit PostDotThread(QObject* parent = nullptr);
    ~PostDotThread();
    void run() override;
    void stopRunning();

    QMap<QString,QFile*> pens;

private:
    bool running = true;

};

#endif // PostDotThread_H
