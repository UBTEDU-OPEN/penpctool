#ifndef SEARCHIPFORMAC_H
#define SEARCHIPFORMAC_H

#include <QObject>
#include <iostream>
#include <QString>
#include <QList>
#include <QHostAddress>
#include <QNetworkInterface>
#include "pingipthread.h"
#include <QRegExp>
#include <QMutex>
#include "devicecommon_global.h"
#include "qsettings.h"
#include <QSettings>
#include <QCoreApplication>
#include "config.h"
#include <QMap>


using namespace std;

class DEVICECOMMON_EXPORT SearchIpForMac: public QObject
{
    Q_OBJECT
private:
    explicit SearchIpForMac(QObject *parent = 0);
    QList<MacAndIp*> macList;
    QList<QString> mMacs;
    QMap<QUuid,PingIpThread*> pingIpThread1;
    QMap<QUuid,PingIpThread*> pingIpThread2;
    QMap<QUuid,PingIpThread*> pingIpThread3;
    QMutex mutex;
    QList<QString> unBufferMacs;
    int macsSize;
    bool isStartSearch;
    QUuid currentId;//代表一次startScarchIp


    QList<MacAndIp*> getSearchAp();
    void stopAllThread(QUuid id);
    void deleteThread(QUuid id);
    QMap<QUuid, std::function<void(QList<MacAndIp*> macList)>> searchResultcallbacks;
    void getBufferIp(QList<QString> macs);

signals:

private slots:
    void getIpForMac(QUuid id, bool isFinished,QString mac,QString ip);

public:
    ~SearchIpForMac();
    static SearchIpForMac* get_instance();
    void startScarchIp(QList<QString> macs,std::function<void(QList<MacAndIp*>)> callBack);
    void stopScarchIp();
    QString getHostIpAddress();

};

#endif // SEARCHIPFORMAC_H
