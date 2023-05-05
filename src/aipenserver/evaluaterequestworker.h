#ifndef WORDDATAWORKER_H
#define WORDDATAWORKER_H

#include <QObject>

#include "charevaluateserver.h"

class EvaluateRequestWorker : public QObject
{
    Q_OBJECT
public:
    explicit EvaluateRequestWorker(QObject *parent = nullptr);

public slots:
    void wordEvaluationRequest(QByteArray wordData);
    void stopRequest();

signals:

private:
    WordDataWriter* writer_;
    bool stopped_ = false;
};

#endif // WORDDATAWORKER_H
