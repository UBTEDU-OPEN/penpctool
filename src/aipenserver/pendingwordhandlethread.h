#ifndef WORDHANDLER_H
#define WORDHANDLER_H

#include <QThread>
#include <QRecursiveMutex>
#include <QList>
#include <atomic>
#include <ctime>

#include "BaseEntity.h"


typedef struct _PendingWordInfo{
    QString mac;
    int bookId;
    int pageId;
    int xIndex;
    int yIndex;
    int zIndex;
    int retryTimes;
    time_t lastTimeStamp;
    std::vector<Stroke>* strokes;
} PendingWordInfo;


class PendingWordHandleThread : public QThread
{
    Q_OBJECT
public:
    explicit PendingWordHandleThread(QObject *parent = nullptr);
    ~PendingWordHandleThread();

    void run() override;
    void stopHandle();

public slots:
    void handleWord(QString mac, int bookId, int pageId, int xIndex, int yIndex, int zIndex, qulonglong strokesPtr);

private:
    QList<PendingWordInfo> pendingWords_;
    QRecursiveMutex dataMutex_;
    std::atomic_bool running_ = true;
};

#endif // WORDHANDLER_H
