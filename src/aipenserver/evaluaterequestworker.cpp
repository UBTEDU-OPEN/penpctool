#include "evaluaterequestworker.h"

#include "logHelper.h"

EvaluateRequestWorker::EvaluateRequestWorker(QObject *parent) : QObject(parent)
{
    writer_ = new WordDataWriter;
}

void EvaluateRequestWorker::wordEvaluationRequest(QByteArray wordData)
{
    LOG(INFO) << "AiPen::wordEvaluate serializeEvaluationRequest stopped_:" << stopped_ /*<< "," << wordData.data()*/;
    if(stopped_) {
        return;
    }
    writer_->writeWordData(wordData);
}

void EvaluateRequestWorker::stopRequest()
{
    stopped_ = true;
    writer_->stopRunning();
}
