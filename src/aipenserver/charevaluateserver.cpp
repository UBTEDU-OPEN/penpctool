#include "charevaluateserver.h"

#include <QBuffer>
#include <QDataStream>
#include <QDebug>

#include "logHelper.h"
#include "projectconst.h"

WordDataWriter::WordDataWriter(QObject *parent) : QObject(parent)
    , wordDataReady_("word_data_ready",0,QSystemSemaphore::Open)
    , wordDataReaded_("word_data_readed",1,QSystemSemaphore::Open)
    , wordSharedData_("word_data")
{

}

void WordDataWriter::writeWordData(const QByteArray& data)
{
    LOG(INFO) << "WordDataWriter::writeWordData begin";
    if(wordDataReaded_.acquire()) {
        LOG(INFO) << "WordDataWriter::writeWordData stopRunning_:" << stopRunning_;
        if(stopRunning_) {
            return;
        }
        LOG(INFO) << "WordDataWriter::writeWordData111";
        if(!wordSharedData_.isAttached()) {
            bool ret = wordSharedData_.create(k10M);
            LOG(INFO) << "WordDataWriter::writeWordData create shared memory ret=:" << ret;
            wordSharedData_.lock();
            memset(wordSharedData_.data(),0,wordSharedData_.size());
            wordSharedData_.unlock();
        }
        LOG(INFO) << "WordDataWriter::writeWordData222";

        QBuffer buffer;
        buffer.open(QBuffer::ReadWrite);
        QDataStream out(&buffer);
        out << data;
        int size = buffer.size();

        wordSharedData_.lock();
        LOG(INFO) << "WordDataWriter::writeWordData locked";
        char *to = (char*)wordSharedData_.data();
        const char *from = buffer.data().data();
        memcpy(to, from, qMin(wordSharedData_.size(), size));
        wordSharedData_.unlock();
        wordDataReady_.release();
        LOG(INFO) << "WordDataWriter::writeWordData post data";
    }
}

void WordDataWriter::stopRunning()
{
    stopRunning_ = true;
    wordDataReaded_.release();
}

WordEvaluateResultReader::WordEvaluateResultReader(QObject *parent) : QObject(parent)
  , evaluateResultReady_("evaluate_result_ready",0,QSystemSemaphore::Open)
  , evaluateResultReaded_("evaluate_result_readed",1,QSystemSemaphore::Open)
  , resultSharedData_("evaluate_result")
{

}


bool WordEvaluateResultReader::readEvaluateResult(EvaluationResponse& response)
{
    LOG(INFO) << "WordEvaluateResultReader::readEvaluateResult begin";
    if(evaluateResultReady_.acquire())
    {
        LOG(INFO) << "WordDataWriter::readEvaluateResult stopRunning_:" << stopRunning_;
        if(stopRunning_) {
            return false;
        }

        if(!resultSharedData_.isAttached()) {
            bool ret = resultSharedData_.attach();
            LOG(INFO) << "WordDataWriter::readEvaluateResult attach memory ret=" << ret;
        }

        QBuffer buffer;
        QDataStream in(&buffer);
        QByteArray bytes;

        resultSharedData_.lock();
        LOG(INFO) << "WordDataWriter::readEvaluateResult locked";
        buffer.setData((char*)resultSharedData_.constData(), resultSharedData_.size());
        buffer.open(QBuffer::ReadOnly);
        in >> bytes;
        memset(resultSharedData_.data(),0,resultSharedData_.size());
        resultSharedData_.unlock();
        evaluateResultReaded_.release();
        LOG(INFO) << "WordDataWriter::readEvaluateResult recv shared data:"/* << bytes.data()*/;
        if(!bytes.toStdString().empty()) {
            unserializeEvaluationResponse(bytes.toStdString(),response);
            LOG(INFO) << "WordDataWriter::readEvaluateResult unserializeEvaluationResponse imgKey="
                      << response.imgKey;
            return true;
        }
    }
    return false;
}

void WordEvaluateResultReader::stopRunning()
{
    LOG(INFO) << "WordDataWriter::stopRunning";
    stopRunning_ = true;
    evaluateResultReady_.release();
}
