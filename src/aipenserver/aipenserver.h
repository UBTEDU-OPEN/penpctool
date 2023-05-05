#ifndef AIPENSERVER_H
#define AIPENSERVER_H

#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QThread>
#include <QMutex>
#include "HttpWorker.h"
#include "IPCProtocol.h"
#include "json/json.h"

class AiPenServer : public QObject
{
    Q_OBJECT
public:
    AiPenServer();
    ~AiPenServer();
#ifdef SHOW_TEST_BTN
    void onOpenOriginFile(QString filePathes);
#endif

signals:
    void sms(QString);
    void stopTimer();
public slots:
    void onPenPower(QString mac, int Battery, int Charging);
    void onPenFirmwareVersion(QString mac, QString FN);
private slots:
    void send(const QString &msg);
    void newConnection();
    void readSocket();
    void readSMS(const QString msg);

    void discardSocket();
private:
    QString buildPowerMsg(QString mac, int Battery, int Charging);
    QString buildFirmwareVersionMsg(QString mac, QString FN);
    void HandleStartClasswork(const Json::Value &value);

private:
    QLocalServer m_server;
    QLocalSocket* m_pConnection;
    QThread httpWorkThread_;
    HttpWorker* httpWorker_;
    QMutex m_mutex;
    QAtomicInt m_msg_id;

};

#endif // AIPENSERVER_H
