#ifndef PINGIPTHREAD_H
#define PINGIPTHREAD_H

#include <QObject>
#include<QThread>
#include<QList>
#include<QString>
#include<qdebug.h>
#include <QtCore/QProcess>
#include<QRegExp>
#include"logHelper.h"
#include <QMutex>
#include <QUuid>

class PingIpThread:public QThread
{
    Q_OBJECT
private:
    QList<QString> mIpList;
    bool isStart;
    QList<QProcess*> mProcessList_;
    QMutex mutex_;
    QUuid id;
public:
    PingIpThread(QUuid id, QList<QString> ipList);
    ~PingIpThread();
    bool getIsStart();
    void stopPing();
signals:
    void ipForMac(QUuid id, bool isFinished,QString mac,QString ip);
protected:
    void run();

};

#endif // PINGIPTHREAD_H
