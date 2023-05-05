#include "pendingwordhandlethread.h"

#include "aipenmanager.h"
#include "logHelper.h"

#include <ctime>

PendingWordHandleThread::PendingWordHandleThread(QObject *parent)
    : QThread(parent)
{

}

PendingWordHandleThread::~PendingWordHandleThread()
{
    LOG(INFO) <<"~PendingWordHandleThread";
}

void PendingWordHandleThread::stopHandle()
{
    running_ = false;
}

void PendingWordHandleThread::run()
{
    bool wordEmpty = true;
    while(running_.load()) {
        {
            QMutexLocker locker(&dataMutex_);
            wordEmpty = pendingWords_.empty();
        }
        if(wordEmpty) {
            QThread::currentThread()->msleep(100);
            continue;
        }

//        qDebug() << "PendingWordHandleThread::run thread id = " << currentThreadId();
        WordDetail detail;
        PendingWordInfo info;
        {
            QMutexLocker locker(&dataMutex_);
            info = pendingWords_.takeFirst();
        }
        time_t currentTimeStamp = time(nullptr);
        if(info.retryTimes > 3 && (info.lastTimeStamp - currentTimeStamp) < 10) {
            {
                QMutexLocker locker(&dataMutex_);
                pendingWords_.push_back(info);
            }
            msleep(100);
            continue;
        }

        ++info.retryTimes;
        info.lastTimeStamp = currentTimeStamp;

        int  wordCode = AiPenManager::getInstance()->getWordDetail(info.bookId,
                                                                   info.pageId,
                                                                   info.zIndex,
                                                                   detail);
        LOG(INFO) << "PendingWordHandleThread::run mac="
                  << info.mac.toStdString() << ",bookId="<<info.bookId << ",pageId=" << info.pageId
                  << ",xIndex=" << info.xIndex << ",yIndex=" << info.yIndex << ",zIndex=" << info.zIndex
                  << ", wordCode=" << wordCode;
        if(0 == wordCode && !AiPenManager::getInstance()->invalidBookId(info.bookId)) {
            {
                QMutexLocker locker(&dataMutex_);
                pendingWords_.push_back(info);
            }
            msleep(100);
            continue;
        }

        if(0 == wordCode) {
            LOG(INFO) << info.mac.toStdString() << "invalid book: " << info.bookId;
        } else {
            if(AiPenManager::getInstance()->getWorkClassId(detail.lessonId) < 0) {
                {
                    QMutexLocker locker(&dataMutex_);
                    pendingWords_.push_back(info);
                }
                msleep(100);
                continue;
            } else {
                AiPen pen(info.mac);
                pen.wordEvaluate(detail,*(info.strokes),AiPenManager::getInstance()->getWorkClassId(detail.lessonId));
            }
        }

        info.strokes->clear();
        delete info.strokes;
    }
}

void PendingWordHandleThread::handleWord(QString mac, int bookId, int pageId, int xIndex,
                                         int yIndex, int zIndex, qulonglong strokesPtr)
{
//    qDebug() << "PendingWordHandleThread::handleWord thread id = " << currentThreadId();
    std::vector<Stroke>* pStroke = reinterpret_cast<std::vector<Stroke>*>(strokesPtr);
    PendingWordInfo info;
    info.mac = mac;
    info.bookId = bookId;
    info.pageId = pageId;
    info.xIndex = xIndex;
    info.yIndex = yIndex;
    info.zIndex = zIndex;
    info.strokes = pStroke;
    info.retryTimes = 0;
    info.lastTimeStamp = time(nullptr);
    {
        QMutexLocker locker(&dataMutex_);
        pendingWords_.push_back(info);
    }
}
