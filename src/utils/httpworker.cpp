#include "httpworker.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include "ubtserver.h"
#include "logHelper.h"
#include "alyservice.h"
#include "httpconfig.h"
//#include "upgradeprocessor.h"

HttpWorker::HttpWorker(QObject *parent) : QObject(parent)
{

}

void HttpWorker::onHttpRequest(QString url, QByteArray data, int type, int funcId, qulonglong tempData)
{
    if(type == 1) //get
    {
        UBTServer::instance()->requestData(url,funcId, tempData);
    } else if(type == 2) { //post
        UBTServer::instance()->postData(url,data,funcId, tempData);
    }
}

void HttpWorker::onReplyResult(int funcId, int httpCode, int result, QByteArray jsonStr, qulonglong tempData)
{
    LOG(INFO) << "HttpWorker::onReplyResult," << funcId << "," << httpCode << "," << result << "," << jsonStr.toStdString();

    QJsonDocument doc = QJsonDocument::fromJson(jsonStr);
    bool isValid = !doc.isNull();
    LOG(INFO) << "HttpWorker::onReplyResult json is valid=" << isValid;

    switch(funcId) {
    case MyRequestType::kRequestAlyInfo:
    {
        if(isValid) {
            QJsonObject obj = doc.object();
            AlyService::instance()->onAlyInfo(result,obj);
        }
    }
        break;
    case MyRequestType::kRequestBookDetail:
    {
        if(isValid) {
            QJsonObject obj = doc.object();
            emit sigGetBookDetail(result,obj,tempData);
        }
    }
        break;
    case MyRequestType::kPostEvaluateResult:
    {
        LOG(INFO) << "post result:" << result;
        emit postResult(result, tempData);
    }
        break;
    case MyRequestType::kRequestSideXml:
    {
        //{"code":0,"message":"success","currentTimes":1638347094767,
        //"data":{"id":1,"type":1,"fileUrl":"AI-pen/test/wordmodel.zip","isDeleted":0,"modifyTime":"2021-11-11T08:15:00.000+0000"}}
        if(0 == result && isValid) {
            QJsonObject obj = doc.object();
            QString modifyTime = obj["data"].toObject()["modifyTime"].toString();
            QString url = obj["data"].toObject()["fileUrl"].toString();
            emit sideXmlResponse(modifyTime,url);
        }
    }
        break;
    case MyRequestType::kRequestXmlDownloadUrl:
    {
        if(isValid) {
            QJsonObject obj = doc.object();
            int* ptr = (int*)tempData;
            UBTServer::instance()->downloadXml(*ptr, obj["data"].toArray().first().toString());
            delete ptr;
        }
    }
        break;
    case MyRequestType::kRequestUpgradeUrl:
    {
        if(isValid) {
            QJsonArray arr = doc.array();
            emit sigHandleUpgradeInfo(result,arr);
        }
    }
        break;
    case MyRequestType::kRequestCharEvaluationUrl:
    {
        if(isValid) {
            QJsonArray arr = doc.array();
            emit sigHandleUpgradeInfo(result,arr);
        }
    }
        break;
    case MyRequestType::kRequestHandWrittenUrl:
    {
        if(isValid) {
            QJsonArray arr = doc.array();
            emit sigHandleUpgradeInfo(result,arr);
        }
    }
        break;
    case MyRequestType::kRequestAipenServerUrl:
    {
        if(isValid) {
            QJsonArray arr = doc.array();
            emit sigHandleUpgradeInfo(result,arr);
        }
    }
        break;
    case MyRequestType::kGetCreateWorkClass:
    {
        if(isValid) {
            QJsonObject obj = doc.object();
            emit sigGetWorkClassId(result,obj,tempData);
        }
    }
        break;
    }
}

void HttpWorker::onRequestDownloadUpgrade(int downloadType, const QString &url, int startPos)
{
    UBTServer::instance()->downloadUpgrade(downloadType,url,startPos);
}

void HttpWorker::onStopTimer()
{
    AlyService::instance()->onStopTimer();
}
