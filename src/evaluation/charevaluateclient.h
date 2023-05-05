#ifndef CHAREVALUATECLIENT_H
#define CHAREVALUATECLIENT_H

#include <QObject>
#include <QSystemSemaphore>
#include <QSharedMemory>
#include <QSharedMemory>

#include "evaluationserializer.h"
#include "charevaluationinterface.h"

class WordDataReader : public QObject
{
    Q_OBJECT
public:
    explicit WordDataReader(QObject *parent = nullptr);
    void readWordData();
    void writeEvaluateResult(const QByteArray& results);

    void onEvaluationRequest(const QByteArray& data);

private:
    QByteArray evaluateResultToJson(const EvaluateReportBody& repoerBody);

private:
    QSystemSemaphore wordDataReady_;
    QSystemSemaphore wordDataReaded_;
    QSharedMemory wordSharedData_;
    QSystemSemaphore evaluateResultReady_;
    QSystemSemaphore evaluateResultReaded_;
    QSharedMemory resultSharedData_;

    CharEvaluationInterface* ceInterface_ = nullptr;
};


#endif // CHAREVALUATECLIENT_H
