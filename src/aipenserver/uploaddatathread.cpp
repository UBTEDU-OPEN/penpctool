#include "uploaddatathread.h"
#include "alyservice.h"
#include "httpconfig.h"
#include "aipenmanager.h"
#include "ubtserver.h"
#include "logHelper.h"

#include <QFile>
#include <QDebug>

#include "config.h"
#include "charevaluateserver.h"

UploadDataThread::UploadDataThread(QObject* parent)
    : QThread(parent)
    , currentIndex_(0)
{
    removeTempFile_ = (0 == Config::getKeepTempFile());
}

UploadDataThread::~UploadDataThread()
{
    LOG(INFO)<<"~UploadDataThread";
}

void UploadDataThread::stopUpload()
{
    uploading = false;
    requestInterruption();
}

void UploadDataThread::run()
{
    bool wordEmpty = true;
    int lastIndex = -1;
    int endIndex = -1;
    while(uploading) {
        {
            QMutexLocker locker(&dataMutex_);
            wordEmpty = preUploadInfo_.empty();
            if(!wordEmpty) {
                endIndex = preUploadInfo_.lastKey();
            } else {
                endIndex = -1;
            }
        }

        if(wordEmpty) {
            QThread::currentThread()->msleep(100);
            continue;
        }

        if(-1 != endIndex && lastIndex >= endIndex) {
            lastIndex = -1; //如果已经是最后一个，重新开始
        }

        qDebug() << "UploadDataThread::run thread id = " << currentThreadId();

        UploadInfo info;
        int index = 0;
        bool allPosted = false;
        time_t currentTimeStamp = time(nullptr);
        bool found = false;
        int postCount = 0;
        {
            QMutexLocker locker(&dataMutex_);
            auto it = preUploadInfo_.begin();
            for(; it != preUploadInfo_.end(); ++it) {
                index = it.key();
                info = it.value();

                if(!info.resultPosted) {
                    if(index > lastIndex) {
                        if(info.retryTimes <= 3 || (info.retryTimes > 3 && (currentTimeStamp - info.lastTimeStamp) >= 10)) {
                            found = true;
                            lastIndex = index;
                            break;
                        }
                    }
                } else {
                    ++postCount;
                }
            }
            if(postCount == preUploadInfo_.size()) {
                allPosted = true;
            }
        }

        if(allPosted || !found) {
            if(!found) {
                lastIndex = -1;
            }
            QThread::currentThread()->msleep(100);
            continue;
        }

        ++info.retryTimes;
        info.lastTimeStamp = currentTimeStamp;

        LOG(INFO) << "UploadDataThread::run mac=" << info.mac.toStdString()
                  << ",img key=" << info.imgKey.toStdString();

        if(!info.imgUploaded) {
            int ret = AlyService::instance()->uploadFile("ubtechinc-common-private",info.imgKey,info.imgPath);
            LOG(INFO) << info.imgKey.toStdString() << ",pic upload ret=" << ret;
            if(1 == ret) {
                info.imgUploaded = true;
                if(removeTempFile_) {
                    QFile::remove(info.imgPath);
                }
            }
        }
        if(ResultType::kOCRResult == info.type) {
            info.jsonUploaded = true;
        }
        if(!info.jsonUploaded) {
            int ret = AlyService::instance()->uploadFile("ubtechinc-common-private",info.jsonKey,info.jsonPath);
            LOG(INFO) << info.jsonKey.toStdString()  << ",upload json ret=" << ret;
            if(1 == ret) {
                info.jsonUploaded = true;
                if(removeTempFile_) {
                    QFile::remove(info.jsonPath);
                }
            }
        }

        LOG(INFO) << info.mac.toStdString() <<  ",upload img=" << info.imgUploaded
                  << ",upload json=" << info.jsonUploaded;

        if(info.imgUploaded && info.jsonUploaded) {
            LOG(INFO) << "start to post: " << info.evaluateResult.toStdString();
            std::string uploadUrl;
            if(ResultType::kOCRResult == info.type) {
                uploadUrl = HttpConfig::instance()->getDictationResult();
            } else {
                uploadUrl = HttpConfig::instance()->getEvaluateResult();
            }

            int* tempData = new int(index);
            emit httpRequest(QString::fromStdString(uploadUrl),info.evaluateResult,2,MyRequestType::kPostEvaluateResult,reinterpret_cast<qulonglong>(tempData));
            info.resultPosted = true;
        }

        {
            QMutexLocker locker(&dataMutex_);
            preUploadInfo_[index] = info;
        }
        //msleep(100);
    }
}

void UploadDataThread::onNewWord(int type, QString mac, QString imgKey, QString imgPath,
                                 QString jsonKey, QString jsonPath, QByteArray evaluateResult)
{
    qDebug() << "UploadDataThread::onNewWord thread id = " << currentThreadId();
    ++currentIndex_;
    UploadInfo info;
    info.type = type;
    info.mac = mac;
    info.imgKey = imgKey;
    info.imgPath = imgPath;
    info.jsonKey = jsonKey;
    info.jsonPath = jsonPath;
    info.evaluateResult = evaluateResult;
    info.imgUploaded = false;
    info.jsonUploaded = false;
    info.resultPosted = false;
    info.retryTimes = 0;
    info.lastTimeStamp = time(nullptr);
    {
        QMutexLocker locker(&dataMutex_);
        preUploadInfo_.insert(currentIndex_,info);
    }
}

void UploadDataThread::onPostResult(int result, qulonglong tempData)
{
    qDebug() << "UploadDataThread::onPostSuccess thread id = " << currentThreadId();
    int* pIndex = reinterpret_cast<int*>(tempData);
    int index = *pIndex;
    delete pIndex;
    {
        QMutexLocker locker(&dataMutex_);
        LOG(INFO) << "UploadDataThread::onPostResult mac="
                  << preUploadInfo_[index].mac.toStdString() << ",imgKey=" << preUploadInfo_[index].imgKey.toStdString()
                   << ",result=" << result;
        if(0 == result) {
            preUploadInfo_.remove(index);
        } else {
            preUploadInfo_[index].resultPosted = false;
        }
    }
}
