#include "readevaluateresultthread.h"

#include "charevaluateserver.h"
#include "logHelper.h"

ReadEvaluateResultThread::ReadEvaluateResultThread(QObject* parent)
    : QThread(parent)
{
}

void ReadEvaluateResultThread::stopRunning()
{
    running = false;
    reader.stopRunning();
}

void ReadEvaluateResultThread::run()
{
    while(running) {
        EvaluationResponse response;
        bool ret = reader.readEvaluateResult(response);
        if(!running) {
            break;
        }
        LOG(INFO) << "ReadEvaluateResultThread::run ret=" << ret;
        if(ret) {
            LOG(INFO) << "ReadEvaluateResultThread::run " << response.mac;
            emit wordResultReady((int)kEvaluationResult, QString::fromStdString(response.mac),
                                 QString::fromStdString(response.imgKey),QString::fromStdString(response.imgPath),
                                 QString::fromStdString(response.jsonKey),QString::fromStdString(response.jsonPath),
                                 QByteArray::fromStdString(response.data));
        }
    }
}
