#include "alyservice.h"

#include <fstream>
#include <QFileInfo>
#include "QJsonDocument"

#include "logHelper.h"
#include "ubtserver.h"
#include "alibabacloud/oss/OssClient.h"
#include "httpconfig.h"

#include <QDebug>

using namespace AlibabaCloud::OSS;
AlyService::AlyService(QObject *parent) : QObject(parent),client(nullptr),
    requested_(false)
{
    LOG(INFO) << "requested=" << requested_.load();
    InitializeSdk();
    timer_ = new QTimer(this);
    connect(timer_,&QTimer::timeout,this,&AlyService::onTimeout);
}

void AlyService::onTimeout()
{
    LOG(INFO) << "AlyService::onTimeout";
    requestToken();
}

void AlyService::onStopTimer()
{
    LOG(INFO) << "AlyService::onStopTimer";
    timer_->stop();
}

void AlyService::restartTimer(bool clientInitResult)
{
    LOG(INFO) << "AlyService::restartTimer clientInitResult=" << clientInitResult;
    if(clientInitResult) {
        timer_->stop();
        timer_->start(1800*1000);
    } else {
        timer_->stop();
        timer_->start(60*1000);
    }
}

void AlyService::onAlyInfo(int result,const QJsonObject& js1)
{
    if(result == 0){
        QMutexLocker locker(&clientMutex_);
        auto accessKeyId = js1["accessKeyId"].toString();
        auto securityToken = js1["securityToken"].toString();
        auto objKeyPrefix = js1["objKeyPrefix"].toString();
        auto bucketName = js1["bucketName"].toString();
        auto endPoint = js1["endPoint"].toString();
        auto accessKeySecret = js1["accessKeySecret"].toString();
        //int expireTimestamp = js1["expireTimestamp"].toInt();
        auto domain = js1["domain"].toString();
        auto expiration = js1["expiration"].toString();
        ClientConfiguration conf;
        qDebug() << "AlyService" << endPoint <<accessKeyId <<accessKeySecret << securityToken << endl;
        if(nullptr != client) {
            LOG(INFO) << "client delete";
            delete client;
            client = nullptr;
        }
        client = new OssClient(endPoint.toStdString(), accessKeyId.toStdString(), accessKeySecret.toStdString(), securityToken.toStdString(), conf);
        LOG(INFO) << "client init success " << std::endl;
        requested_ = false; //只有init成功才再次置为false，其他情况靠timer
//            QString key = "AI-pen/test/qqqqqq";
//            downloadFile("ubtechinc-common-private",key,"E:/new_path/11111111.xml");
    }else{
       LOG(INFO) << "AlyService" << "client init failed error = " << result;

    }
    bool clientInitSucc = (result == 0);
    restartTimer(clientInitSucc);
}

void AlyService::requestToken()
{
    std::string alyurl = HttpConfig::instance()->getUpdataFile();
    QString qAlyUrl = QString::fromStdString(alyurl);
    QString productName = "AI-Pen";
    QString bucket = "2";
    QString alyUrl = qAlyUrl + "?productName=" + productName + "&bucketType=" + bucket + "&validTime=3600";
    LOG(INFO) << "requestToken alyUrl = " << alyUrl.toStdString();
    emit httpRequest(alyUrl,QByteArray(),1,MyRequestType::kRequestAlyInfo,0);
}

void AlyService::init(){
    requestToken();
    timer_->start(1800*1000);
}

AlyService::~AlyService()
{ 
    qDebug()<<"~AlyService()";
//    timer_->stop();
    ShutdownSdk();
    if(client != nullptr){
        delete client;
        client = nullptr;
    }
//    timer_->deleteLater();
}

AlyService* AlyService::instance()
{
    static AlyService* inst = new AlyService;
    return inst;
}

int AlyService::uploadFile(const QString& BucketName,const QString& fileKey,const QString& filePath)

{
    LOG(INFO) << "AlyService::uploadFile " << fileKey.toStdString() << "," << filePath.toStdString();
    QMutexLocker locker(&clientMutex_);
    if(client == nullptr){
        LOG(INFO) << "client init error " << std::endl;
        if(!requested_.load()) { //避免频繁请求
            requested_ = true;
            requestToken();
        }
        return false;
    }
    /* fileKey 表示上传文件到OSS时需要指定包含文件后缀在内的完整路径，例如abc/efg/123.jpg*/
    std::shared_ptr<std::iostream> content = std::make_shared<std::fstream>(filePath.toStdString(), std::ios::in | std::ios::binary);
    PutObjectRequest request(BucketName.toStdString(), fileKey.toStdString(), content);
    /*（可选）请参见如下示例设置存储类型及访问权限ACL*/
    auto outcome = client->PutObject(request);
    LOG(INFO) << "outcome result  = " << outcome.isSuccess();
    if (!outcome.isSuccess()) {
        /* 异常处理 */
        LOG(WARNING) << "PutObject fail" <<
                     ",code:" << outcome.error().Code() <<
                     ",message:" << outcome.error().Message() <<
                     ",requestId:" << outcome.error().RequestId()<<
                     ",filePath :" << filePath.toStdString();
        if(!requested_.load()) { //避免频繁请求
            requested_ = true;
            requestToken();
        }
    }else{
        LOG(INFO) << "PutObject Success"<<
                     ",filePath :" << filePath.toStdString();
    }
    return outcome.isSuccess();
}


int AlyService::downloadFile(const QString& BucketName,const QString& fileKey,const QString& filePath){
    QMutexLocker locker(&clientMutex_);
    if(client == nullptr){
        LOG(INFO) << "client init error " << std::endl;
    }
     GetObjectRequest request(BucketName.toStdString(), fileKey.toStdString());
     request.setResponseStreamFactory([filePath]() {
            return std::make_shared<std::fstream>(
                            filePath.toStdString(),
                            std::ios_base::out | std::ios_base::in | std::ios_base::trunc | std::ios_base::binary
                        );
        }
     );
     auto outcome = client->GetObject(request);
     return outcome.isSuccess();
}
