#ifndef CHAREVALUATESERVER_H
#define CHAREVALUATESERVER_H

#include <QObject>
#include <QSystemSemaphore>
#include <QSharedMemory>
#include <QSharedMemory>

#include "evaluationserializer.h"


class WordDataWriter : public QObject
{
    Q_OBJECT
public:
    explicit WordDataWriter(QObject *parent = nullptr);
    void writeWordData(const QByteArray& data);
    void stopRunning();

private:
    QSystemSemaphore wordDataReady_;
    QSystemSemaphore wordDataReaded_;
    QSharedMemory wordSharedData_;
    bool stopRunning_ = false;
};

class WordEvaluateResultReader : public QObject
{
    Q_OBJECT
public:
    explicit WordEvaluateResultReader(QObject *parent = nullptr);

    bool readEvaluateResult(EvaluationResponse& response);
    void stopRunning();

signals:

private:
    QSystemSemaphore evaluateResultReady_;
    QSystemSemaphore evaluateResultReaded_;
    QSharedMemory resultSharedData_;
    bool stopRunning_ = false;
};

#endif // CHAREVALUATESERVER_H
