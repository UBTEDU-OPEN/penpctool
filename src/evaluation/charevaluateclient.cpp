#include "charevaluateclient.h"

#include <QBuffer>
#include <QDataStream>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QArrayData>
#include <QFile>
#include <json/json.h>
#include <QPluginLoader>
#include <QCoreApplication>

#include "logHelper.h"
//#include "httpconfig.h"
#include "projectconst.h"
#include "config.h"

WordDataReader::WordDataReader(QObject *parent) : QObject(parent)
  , wordDataReady_("word_data_ready",0,QSystemSemaphore::Open)
  , wordDataReaded_("word_data_readed",1,QSystemSemaphore::Open)
  , wordSharedData_("word_data")
  , evaluateResultReady_("evaluate_result_ready",0,QSystemSemaphore::Open)
  , evaluateResultReaded_("evaluate_result_readed",1,QSystemSemaphore::Open)
  , resultSharedData_("evaluate_result")
{
    QPluginLoader loader("charevaluationplugin.dll");
    auto object = loader.instance();
    LOG(INFO) << "TEST INSTANCE=" << object << ", error info:" << loader.errorString().toStdString();
    ceInterface_ = qobject_cast<CharEvaluationInterface*>(object);
    QJsonObject json = loader.metaData().value("MetaData").toObject();
    LOG(INFO) << "";
    LOG(INFO) << "********** MetaData **********";
    LOG(INFO) << json.value("type").toInt();
    LOG(INFO) << json.value("update_time").toString().toStdString();
    LOG(INFO) << json.value("name").toString().toStdString();
    LOG(INFO) << json.value("version").toString().toStdString();
    LOG(INFO) << "ceInterface_=" << ceInterface_;
}

void WordDataReader::readWordData()
{
    LOG(INFO) << "WordDataReader::readWordData";
    if(wordDataReady_.acquire())
    {
        LOG(INFO) << "WordDataReader::readWordData got data ready";
        if(!wordSharedData_.isAttached()) {
            LOG(INFO) << "WordDataReader::readWordData attach memory 1111";
            bool ret = wordSharedData_.attach();
            LOG(INFO) << "WordDataReader::readWordData attach memory ret=" << ret;
        }

        QBuffer buffer;
        QDataStream in(&buffer);
        QByteArray wordJson;
        wordSharedData_.lock();
        LOG(INFO) << "WordDataReader::readWordData locked";
        buffer.setData((char*)wordSharedData_.constData(), wordSharedData_.size());
        buffer.open(QBuffer::ReadOnly);
        in >> wordJson;
        memset(wordSharedData_.data(),0,wordSharedData_.size());
        wordSharedData_.unlock();
        LOG(INFO) << "WordDataReader::readWordData recv shared data:" /*<< wordJson.data()*/;
        wordDataReaded_.release();
        if(!wordJson.toStdString().empty()) {
            onEvaluationRequest(wordJson);
        }
        LOG(INFO) << "WordDataReader::readWordData released word data readed";
    }
}

void WordDataReader::writeEvaluateResult(const QByteArray& results)
{
    LOG(INFO) << "WordEvaluateResultWriter::writeEvaluateResult begin";
    if(evaluateResultReaded_.acquire()) {
        LOG(INFO) << "WordEvaluateResultWriter::writeEvaluateResult got result readed";
        if(!resultSharedData_.isAttached()) {
            bool ret = resultSharedData_.create(k10M);
            LOG(WARNING) << "WordEvaluateResultWriter::writeEvaluateResult create shared memory ret=" << ret
                         << "," << resultSharedData_.error();
            if(QSharedMemory::AlreadyExists == resultSharedData_.error()) {
                ret = resultSharedData_.attach();
                LOG(WARNING) << "WordEvaluateResultWriter::writeEvaluateResult attach ret=" << ret
                             << "," << resultSharedData_.error();
            } else {
                LOG(INFO) << "WordEvaluateResultWriter::writeEvaluateResult clear share memory";
                resultSharedData_.lock();
                memset(resultSharedData_.data(),0,resultSharedData_.size());
                resultSharedData_.unlock();
            }
        }

        QBuffer buffer;
        buffer.open(QBuffer::ReadWrite);
        QDataStream out(&buffer);
        out << results;
        int size = buffer.size();

        LOG(INFO) << "WordEvaluateResultWriter::writeEvaluateResult begin lock";
        resultSharedData_.lock();
        LOG(INFO) << "WordEvaluateResultWriter::writeEvaluateResult locked";
        char *to = (char*)resultSharedData_.data();
        const char *from = buffer.data().data();
        memcpy(to, from, qMin(resultSharedData_.size(), size));
        resultSharedData_.unlock();
        evaluateResultReady_.release();
        LOG(INFO) << "WordEvaluateResultWriter::writeEvaluateResult wrote data";
    }
}


void WordDataReader::onEvaluationRequest(const QByteArray& data)
{
    if(nullptr == ceInterface_) {
        LOG(WARNING) << "char evaluation is null";
        return;
    }
    LOG(INFO) << "EvaluationRequestWorker::onEvaluationRequest111";
    EvaluationRequest request;
    unserializeEvaluationRequest(data.toStdString(),request);
    LOG(INFO) << "EvaluationRequestWorker::onEvaluationRequest222 " << request.imgKey;
    EvaluateReportBody body;
    ceInterface_->evaluate(request,body);
    auto bytes =  evaluateResultToJson(body);

    LOG(INFO) << "CharEvaluateClient::wordEvaluate saveResult=" << bytes.data();

    auto results = serializeEvaluationResponse(request.mac,request.imgKey,request.imgPath,request.jsonKey,request.jsonPath,bytes.toStdString());
    writeEvaluateResult(QByteArray::fromStdString(results));
}

QByteArray WordDataReader::evaluateResultToJson(const EvaluateReportBody& repoerBody)
{
    QJsonDocument jdoc;
    QJsonObject baseObj;
    QJsonObject obj;
    QJsonArray arr;
    if(repoerBody.practiseBriefInfo.empty())
    {
        QJsonDocument jdoc1;
        QJsonObject obj1;
        obj1["id"] = 0;
        obj1["lessonId"] = repoerBody.lessonId;
        obj1["wordId"] = repoerBody.contentId;
        obj1["wordCode"] = repoerBody.contentCode;
        obj1["createTime"] = 500;
        obj1["evaluateScore"] = repoerBody.score;
        obj1["isCorrect"] = repoerBody.isCorrect == 1 ? true : false;
        obj1["strokeNum"] = repoerBody.strokeNumCorrect;
        if(repoerBody.workClassId != -1) {
            obj1["workId"] = repoerBody.workClassId;
        } else {
            obj1["workId"] = 0;
        }
        jdoc1.setObject(obj1);
        obj["practiseBriefInfo"] = QString(jdoc1.toJson());
    } else {
        obj["practiseBriefInfo"] = QString::fromStdString(repoerBody.practiseBriefInfo);
    }
    obj["contentCode"] = repoerBody.contentCode;
    obj["contentId"] = repoerBody.contentId;
    obj["contentName"] = QString::fromStdString(repoerBody.contentName);
    obj["isCorrect"] = repoerBody.isCorrect;
    obj["lessonId"] = repoerBody.lessonId;
    obj["practiseType"] = repoerBody.practiseType;
    obj["score"] = repoerBody.score;
    if(repoerBody.workClassId != -1) {
        obj["workClassId"] = repoerBody.workClassId;
    }
    obj["practiseXmlUrl"] = QString::fromStdString(repoerBody.practiseXmlUrl);
    obj["thumbnailUrl"] = QString::fromStdString(repoerBody.thumbnailUrl);
    for(int i=0;i<repoerBody.errList.size();i++)
        {
            QJsonObject Member;     //定义数组成员
            Member["errorType"] = repoerBody.errList.at(i).errorType;
            Member["errorCode"] = QString::fromStdString(repoerBody.errList.at(i).errorCode);
            Member["errorScore"] = repoerBody.errList.at(i).errorScore;
            arr.append(Member);
        }
    obj["errList"] = arr;
    if(-1 != repoerBody.wordOrder) {
        obj["wordOrder"] = repoerBody.wordOrder;
    }
    baseObj["practiseResultDTO"] = obj;
    baseObj["mac"] = QString::fromStdString(repoerBody.mac);
    jdoc.setObject(baseObj);
    QByteArray bytes = jdoc.toJson(QJsonDocument::Indented);
    LOG(INFO) << "AiPen::evaluateResultToJson data=" << bytes.data();

    return bytes;
}


