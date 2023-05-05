#ifndef READEVALUATERESULTTHREAD_H
#define READEVALUATERESULTTHREAD_H

#include <QThread>

#include "charevaluateserver.h"
#include "uploaddatathread.h"

class ReadEvaluateResultThread : public QThread
{
    Q_OBJECT
public:
    explicit ReadEvaluateResultThread(QObject* parent = nullptr);
    void stopRunning();

    void run() override;

signals:
    void wordResultReady(int type, QString mac, QString imgKey, QString imgPath,
                         QString jsonKey, QString jsonPath, QByteArray evaluateResult);

private:
    volatile bool running = true;
    WordEvaluateResultReader reader;
};

#endif // READEVALUATERESULTTHREAD_H
