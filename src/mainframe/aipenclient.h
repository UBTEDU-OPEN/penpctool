#ifndef AIPENCLIENT_H
#define AIPENCLIENT_H

#include <QLocalSocket>
#include <QMutex>
#include <QTimer>

#include "Classwork.h"
#include "json/json.h"

class AiPenLocalClient : public QObject
{
    Q_OBJECT
public:
    AiPenLocalClient();
    ~AiPenLocalClient();

public:
    void finishAllClassWork();
#ifdef SHOW_TEST_BTN
    void openOriginFile(QString filePathes);
    QString buildOpenOriginFile(QString filePathes);
#endif

public slots:
    void onNotifyAttendClass(int classId);
    void onNotifyFinishClass();
    void onNotifyStartClasswork(QString strClasswork);
    void onNotifyStopClasswork();
    void onNotifyStartDictation(int workClassId);
    void onNotifyStopDictation();
    void onDictationWordStart(QString msg);
    void onTimeout();
    void onNoFN(QString mac, QString ip);


signals:
    void sms(QString);
private slots:
    bool send(const QString &msg);

    void readSocket();
    void readSMS(QString);

    void onConnectSocket();
    void discardSocket();
private:
    QString buildBaseMsg(int nCommand);
    QString buildAttendClass(int classId);
    QString buildStartClasswork(const Json::Value &value);
    QString buildStartDictation(int workClassId);
    QString buildDictationWord(QString msg);

private:
    QLocalSocket* conection = nullptr;
    QAtomicInt m_msg_id;
    QMutex m_mutex;
    bool m_bCon = false;
    bool finishAllPending_ = false;
    QTimer* connectTimer_ = nullptr;
};

#endif // AIPENCLIENT_H
