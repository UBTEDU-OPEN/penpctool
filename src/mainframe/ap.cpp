#include "ap.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QNetworkInterface>
#include <json/json.h>
#include <QTimer>
#include <QThread>
#include <QDebug>
#include "logHelper.h"
#include "TQLAPComm.h"


using namespace std;
using namespace Json;


Ap::Ap(QString ip, QString mac, QObject *parent):
    QObject(parent),
    ip_(ip),
    mac_(mac),
    setuped(false),
    setupTimer_(nullptr),
    startNotify(false),
    m_reply(nullptr),
    sseReconnecting_(0)
{
    apInfo_ = new APInfo();
    serverInfo_ = new ServerInfo();
    apInfo_->ip = ip;
    apInfo_->mac = mac;
    manager_ = new QNetworkAccessManager();
    manager_->configuration().setConnectTimeout(3*1000);
    isGetApInfoing_ .fetchAndStoreAcquire(1);
    needStartSseTimer_.fetchAndStoreAcquire(1);
}

Ap::~Ap(){
    stopEventObserver();
    if (nullptr != m_reply)
    {
        m_reply->deleteLater();
        m_reply = nullptr;
    }
    if (nullptr != setupTimer_ && isGetApInfoing_.testAndSetAcquire(0, 1))
    {
        setupTimer_->stop();
        setupTimer_->deleteLater();
        setupTimer_ = nullptr;
    }

    delete manager_;

    delete apInfo_;
    delete serverInfo_;

    qDeleteAll(pairedPenList_);
    qDeleteAll(scanedPenList_);
    pairedPenList_.clear();
    scanedPenList_.clear();
}

bool Ap::isSetuped()
{
    return setuped;
}

APInfo Ap::getAPInfo()
{
    return *apInfo_;
}

ServerInfo Ap::getServerInfo()
{
    return *serverInfo_;
}

int Ap::updatePenPower(QString mac, int Battery, int Charging)
{
    if(pairedPenList_.contains(mac))
    {
        PenInfo* p = pairedPenList_[mac];
//        if(p->charging != Charging || p->battery != Battery) {
            p->charging = Charging;
            p->battery = Battery;
            emit PenPowerChanged(*p);
//        }
        //LOG(INFO) << "Ap::updatePenPower FN=" << p->FN.toStdString();
        if(p->FN.isEmpty()) {
            emit noFN(mac,ip_);
        }
        bool bFirst = false; //因为增加了恢复数据，所以首次使用已连接的话，无论有没有连接都通知一下给web
        if (!m_penConnectNotifyList.contains(mac))
        {
            m_penConnectNotifyList.append(mac);//未通知过的笔连接状态加入到已通知列表
            bFirst = true;
        }
        if(p->connected == 0 || bFirst) {
            p->connected = 1;
            p->paired = 1;
            //增加连上的笔的监听
            LOG(INFO)<<"updatePenPower->SSEConnectStatusData: pen.mac="<<p->mac.toStdString();
            emit SSEConnectStatusData(*p);
        }
        return 0;
    }
    return -1;
}


int Ap::updatePenFirmwareVersion(QString mac, QString FN)
{
    for(PenInfo *p: pairedPenList_){
        if (p->mac == mac){
            p->FN = FN;
            return 0;
        }
    }
    return -1;
}

void Ap::setup(const ServerInfo& info, std::function<void(int code, QString msg)> callback)
{
    serverInfo_->ip = info.ip;
    serverInfo_->port = info.port;
    serverInfo_->type = info.type;
    serverInfo_->proto = info.proto;
    //可能存在线程安全问题和内存释放问题
    qDeleteAll(pairedPenList_);
    qDeleteAll(scanedPenList_);
    pairedPenList_.clear();
    scanedPenList_.clear();
    stopEventObserver();
    m_retries = 0;
    getServerInfo([callback, this](int code, QString msg, ServerInfo* response){
        if(code == 0){
            if(serverInfo_->ip == response->ip && serverInfo_->port == response->port) {
                //同一台pc,不需要设置Serverinfo
                this->getApInfo([callback, this](int code ,QString msg, APInfo* apInfo) {
                    if(code ==0){
                        this->apInfo_ = apInfo;
                        this->setuped = true;
                        qDebug() <<"AP:"<< apInfo->mac<<QStringLiteral("1.init success.")<< endl;
                        callback(code, msg);
                        this->startScan(nullptr);
                        this->startEventObserver();
                    }else{
                        this->setuped = false;
                        LOG(INFO)<<QString::fromLocal8Bit("1.查询APInfo失败").toStdString()<<endl;
                        callback(code, msg);
                    }
                });
            }else {
                //不同PC, 需要设置Serverinfo
                setServerInfo(*serverInfo_, [callback, this](int code, QString msg){
                    if (code == 0) {
                       if(setupTimer_) {
                           LOG(ERROR)<<"setup->setServerInfo->setupTimer has started."<<endl;
                           callback(500, "setup->setServerInfo->setupTimer has started.");
                           return;
                       }
                       setupTimer_ = new QTimer(this);
                       connect(setupTimer_, &QTimer::timeout, [callback, this](){
                           if (isGetApInfoing_.testAndSetAcquire(1, 0)) {
                               this->getApInfo([this, callback](int code ,QString msg, APInfo* apInfo) {
                                   if(code ==0 && setupTimer_){
                                       setupTimer_->stop();
                                       setupTimer_->deleteLater();
                                       setupTimer_ = nullptr;

                                       this->apInfo_ = apInfo;
                                       this->setuped = true;
                                       callback(code, msg);
                                       //qDebug() <<"getApInfo success."<< endl;
                                       this->startScan(nullptr);
                                       this->startEventObserver();

                                   }else{
                                       LOG(INFO)<<"getApInfo error: "<<msg.toStdString()<<endl;
                                   }
                                   isGetApInfoing_.storeRelaxed(1);
                               });
                           }
                       });
                       setupTimer_->start(6000);
                    }else {
                        LOG(INFO)<<QString::fromLocal8Bit("2.配置AP ServerInfo失败").toStdString()<<endl;
                        this->setuped = false;
                        callback(code, msg);
                    }
                });
            }
        }else{
            this->setuped = false;
            LOG(INFO)<<QString::fromLocal8Bit("查询AP配置失败").toStdString()<<endl;
            callback(code, msg);
        }
    });
}

/**
 * curl --location --request POST '/api/scan'
 * {"code":0,"msg":"success","data":{}}
 * @brief AP::startScan
 * @return
 */
void Ap::startScan(std::function<void(int code, QString msg)> callback)
{
    doPost("/api/scan", "", [callback](JsonResponse response){
        if (nullptr != callback)
            callback(response.code, response.msg);
    });
}

/**
 * curl --location --request POST '/api/scan/stopScan'
 * {"code":0,"msg":"success","data":{}}
 * @brief AP::stopScan
 * @return
 */
void Ap::stopScan(std::function<void(int code, QString msg)> callback)
{
    doPost("/api/scan/stopScan", "", [callback](JsonResponse response){
        if (nullptr != callback)
            callback(response.code, response.msg);
    });
}

void Ap::getScanStatus(std::function<void (int, QString)> callback)
{
    doGet("/api/scan",[callback](JsonResponse response){
        if(nullptr != callback){
            callback(response.code,response.msg);
        }
    });
}

/**
 * curl --location --request GET '/api/connection'
 * {"code":0,"msg":"success","data":{"total":29,"conn":0,"noconn":29,"list":[{"mac":"D8:0B:CB:61:1B:E6","name":"SmartPen","flag":"TQL-130","connected":0,"battery":-1,"charging":-1},
 * @brief AP::getPairedPens
 * @param pens
 * @return
 */
void Ap::getPairedPens(std::function<void(int code, QString msg, QList<PenInfo> pens)> callback)
{
    doGet("/api/connection", [callback, this](JsonResponse response) {
        if(response.code != 0) {
            QList<PenInfo> empty;
            if (nullptr != callback)
                callback(response.code, response.msg, empty);
            return ;
        }
        QList<PenInfo> pens;
        Json::Value data = response.data;
        Json::Value rawPenList = data["list"];
        for (unsigned int i = 0; i < rawPenList.size(); ++i)
        {
            QString mac = rawPenList[i]["mac"].asCString();
            PenInfo* pen = nullptr;
            //更新缓存,数据可能存在线程安全问题
            if (this->pairedPenList_.contains(mac)) {
                pen = this->pairedPenList_[mac];
            }else {
                pen = new PenInfo;
                pen->mac = mac;
                this->pairedPenList_.insert(mac, pen);
            }
            pen->name = rawPenList[i]["name"].asCString();
            pen->flag = rawPenList[i]["flag"].asCString();
            pen->connected = rawPenList[i]["connected"].asInt();
            //pen->battery = rawPenList[i]["battery"].asInt();
            //pen->charging = rawPenList[i]["charging"].asInt();
            pens.push_back(*pen);
        }
        if (nullptr != callback)
            callback(response.code, response.msg, pens);
    });
}

/**
 * curl --location --request GET '/api/scan/list'
 * @brief AP::getUnpairedPens
 * @param pens
 * @return
 */
void Ap::getUnpairedPens(std::function<void(int code, QString msg, QList<PenInfo> pens)> callback)
{
    doGet("/api/scan/list", [callback, this](JsonResponse response) {
        if(response.code != 0) {
            QList<PenInfo> empty;
            if (nullptr != callback)
                callback(response.code, response.msg, empty);
            return ;
        }
        QList<PenInfo> pens;
        Json::Value data = response.data;
        Json::Value rawPenList = data["list"];
        for (unsigned int i = 0; i < rawPenList.size(); ++i)
        {

            QString mac = rawPenList[i]["mac"].asCString();
            PenInfo* pen = nullptr;
            //更新缓存,数据可能存在线程安全问题
            if (this->scanedPenList_.contains(mac)) {
                pen = this->scanedPenList_[mac];
            }else {
                pen = new PenInfo;
                pen->mac = mac;
                this->scanedPenList_.insert(mac, pen);
            }
            pen->name = rawPenList[i]["name"].asCString();
            pen->flag = rawPenList[i]["flag"].asCString();
            pen->timestamp = rawPenList[i]["timestamp"].asUInt64();
            pen->connected = 0;//默认未链接， 默认未配对
            pens.push_back(*pen);
        }
        if (nullptr != callback)
            callback(response.code, response.msg, pens);
    });
}

/**
 * copy all pens: paired/unpaired/connected/unconnected
 * @brief Ap::getAllPens
 * @return
 */
QList<PenInfo> Ap::getAllPens()
{
    if (!setuped) return QList<PenInfo>();
    QList<PenInfo*> values = pairedPenList_.values();
    QList<PenInfo*>::iterator iter = values.begin();
    QList<PenInfo> ret;
    for(; iter != values.end(); ++iter){
        PenInfo info = **iter;
        info.paired = 1;
        ret.push_back(info);
    }
    values = scanedPenList_.values();
    iter = values.begin();
    for(; iter != values.end(); ++iter){
        PenInfo info = **iter;
        info.paired = 0;
        ret.push_back(info);
    }
    return ret;
}

/** copy current paired pen
 * @brief AP::getCurrentPairedPenMacList
 * @return
 */
QList<QString> Ap::getCurrentPairedPenMacList()
{
    if (!setuped) return QList<QString>();
    QList<PenInfo*> values = pairedPenList_.values();
    QList<QString> ret;
    for(PenInfo* pInfo : values){
        PenInfo info = *pInfo;
        info.paired = 1;
        ret.push_back(info.mac);
    }
    return ret;
}

/**
 * curl --location --request POST '/api/connection' --header 'Content-Type:application/json' --data-raw '{"macList":["D8:0B:CB:61:1C:49"]}'
 * {"code":0,"msg":"success","data":{"invalidMacList":["10:12:44:sd:ss:11"]}}
 * @brief AP::pairPens
 * @param pen_macs
 * @return
 */
void Ap::pairPens(QList<QString> pen_macs, std::function<void(int code, QString msg)> callback)
{
    QString postParams = "{\"macList\":[";
    QList<QString>::const_iterator iter = pen_macs.cbegin();
    for(int i=1; iter != pen_macs.cend(); ++iter, i++)
    {
        postParams.append("\"");
        postParams.append(*iter);
        if (i != pen_macs.length())
            postParams.append("\",");
        else
            postParams.append("\"");
    }
    postParams.append("]}");
    qDebug() <<"pairPens: "<<postParams;
    doPost("/api/connection", postParams, [this,callback, pen_macs](JsonResponse response) {
        if (response.code == 0) {
            for(const QString &mac: pen_macs) {
                //更新缓存,数据可能存在线程安全问题
                PenInfo* pen = nullptr;
                if (!this->pairedPenList_.contains(mac)) {
                    pen = new PenInfo;
                    pen->mac = mac;
                    pen->paired = 1;
                    this->pairedPenList_.insert(mac, pen);
                }
            }
        }
        if (nullptr != callback)
            callback(response.code, response.msg);
    });
}

/**
 * curl --location --request POST '/api/connection/unbind' --header 'Content-Type:application/json' --data-raw '{"macList":["D8:0B:CB:61:1C:49", "10:12:44:sd:ss:11"]}'
 * {"code":0,"msg":"success","data":{"invalidMacList":["10:12:44:sd:ss:11"]}}
 * @brief AP::unpairPens
 * @param pen_macs
 * @return
 */
void Ap::unpairPens(QList<QString> pen_macs, std::function<void(int code, QString msg)> callback)
{
    QString postParams = "{\"macList\":[";
    QList<QString>::const_iterator iter = pen_macs.cbegin();
    for(int i = 1; iter != pen_macs.cend(); ++iter, i++)
    {
        postParams.append("\"");
        postParams.append(*iter);
        if (i != pen_macs.length())
            postParams.append("\",");
        else
            postParams.append("\"");
    }
    postParams.append("]}");
    qDebug()<<"unpairPens:"<<postParams;
    for(QString &mac: pen_macs) {
        delete pairedPenList_.take(mac);
    }
    doPost("/api/connection/unbind", postParams, [callback](JsonResponse response) {
        if (nullptr != callback)
            callback(response.code, response.msg);
    });
}

/**
 *  curl --locaiton --request POST '/api/scan/clearList' --header 'Content-Type:application/json' --data-raw '{"macList":["D8:0B:CB:61:1C:49"]}
 *  curl --locaiton --request POST '/api/scan/clearList?all=1'
 * @brief AP::clear_unpaired_pens
 * @param pen_macs
 * @return
 */
void Ap::clearUnpairedPens(QList<QString> pen_macs, std::function<void(int code, QString msg)> callback)
{
    QString postParams = "{\"macList\":[";
    QList<QString>::iterator iter = pen_macs.begin();
    for(int i = 1; iter != pen_macs.end(); ++iter, i++)
    {
        postParams.append("\"");
        postParams.append(*iter);
        if (i != pen_macs.length())
            postParams.append("\",");
        else
            postParams.append("\"");
    }
    postParams.append("]}");
    QString api = "/api/scan/clearList";
    doPost(api +(pen_macs.size()== 0? "?all=1":""), (pen_macs.size()== 0? "": postParams), [callback](JsonResponse response) {
        if (nullptr != callback)
            callback(response.code, response.msg);
    });
}

void Ap::clearAllUnpairPens(std::function<void (int, QString)> callback)
{
    doPost("/api/scan/clearList?all=1","",[callback](JsonResponse response){
        if (nullptr != callback)
            callback(response.code, response.msg);
    });
}


/**
 * curl --location --request GET '/api/ap/reboot'
 * @brief AP::reboot
 * @return
 */
void Ap::reboot(std::function<void(int code, QString msg)> callback)
{
    doGet("/api/ap/reboot", [callback](JsonResponse response){
        if (nullptr != callback)
            callback(response.code, response.msg);
    });
}

//*******************private***************************

/**
 *  POST 'http:///api/report-way' --header 'Content-Type:application/json' --data-raw '{"ip":"10.10.31.97", "port":8888, "proto":"tcp", "type":"ap0"}'
 *  {"code":0,"msg":"success","data":{}}
 * @brief AP::setServerInfo
 * @param info
 * @return
 */
void Ap::setServerInfo(const ServerInfo& info, std::function<void(int code, QString msg)> callback)
{
    QString postParams;
    postParams.append("{\"ip\":\"").append(info.ip).append("\",");
    postParams.append("\"port\":").append(QString::number(info.port, 10)).append(",");
    postParams.append("\"proto\":\"").append(info.proto).append("\",");
    postParams.append("\"type\":\"").append(info.type).append("\"}");

    doPost("/api/report-way", postParams, [callback](JsonResponse response){
        if (nullptr != callback)
            callback(response.code, response.msg);
    });
}

/**
 * curl --location --request GET '/api/report-way'
 * {"code":0,"msg":"success","data":{"ip":"10.10.31.97","port":8888,"proto":"tcp","type":"ap0"}}
 * @brief AP::getServerInfo
 * @param callback
 * @return
 */
void Ap::getServerInfo(std::function<void(int code, QString msg, ServerInfo* response)> callback)
{
    doGet("/api/report-way", [callback](JsonResponse response){
        if (response.code !=0 ){
            if (nullptr != callback)
                callback(response.code, response.msg, nullptr);
            return;
        }
        if (nullptr != callback) {
            ServerInfo* info = new ServerInfo();
            info->ip = response.data["ip"].asCString();
            if(response.data["port"].isInt()){
                info->port = response.data["port"].asInt();
            }else {
                const char* p = response.data["port"].asCString();
                info->port = atoi(p);
            }

            info->proto = response.data["proto"].asCString();
            info->type = response.data["type"].asCString();
            callback(response.code, response.msg, info);
        }
    });
}

/**
 *  GET '/api/app-info'
 *   {"code":0,"msg":"success","data":{
 *           "ip":"10.10.31.109",
 *           "netmask":"255.255.255.0",
 *           "mac":"CC:1B:E0:E1:BE:44",
 *           "fwVersion":"2.1.0.2104151910",
 *           "appVersion":"3.1",
 *           "connected":{
 *                       "total":29,
 *                       "conn":0,
 *                       "noconn":29,
 *                       "list":[
 *                           {"mac":"D8:0B:CB:61:1B:E6","name":"SmartPen","flag":"TQL-130","connected":0,"battery":-1,"charging":-1}]
 *                       }
 *                   }
 * @brief AP::get_detail_info
 * @return code
 */
void Ap::getApInfo(std::function<void(int code, QString msg, APInfo* response)> callback)
{
    doGet("/api/app-info",[callback, this](JsonResponse response){
        if (response.code !=0 ){
            if (nullptr != callback)
                callback(response.code, response.msg, nullptr);
            return;
        }
        APInfo* info = new APInfo();
        info->ip = response.data["ip"].asCString();
        info->mac = response.data["mac"].asCString();
        info->netmask = response.data["netmask"].asCString();
        info->fwVersion = response.data["fwVersion"].asCString();
        info->appVersion = response.data["appVersion"].asCString();

        //直接取出该AP配对的笔, //更新缓存, 数据可能存在线程安全问题
        Json::Value rawPenList = response.data["connected"]["list"];
        for (unsigned int i = 0; i < rawPenList.size(); ++i)
        {
            QString mac = rawPenList[i]["mac"].asCString();
            PenInfo* pen = nullptr;
            //更新缓存,数据可能存在线程安全问题
            if (this->pairedPenList_.contains(mac)) {
                pen = this->pairedPenList_[mac];
            }else {
                pen = new PenInfo;
                pen->mac = mac;
                this->pairedPenList_.insert(mac, pen);
            }
            pen->name = rawPenList[i]["name"].asCString();
            pen->flag = rawPenList[i]["flag"].asCString();
            pen->connected = rawPenList[i]["connected"].asInt();
            //pen->battery = rawPenList[i]["battery"].asInt();
            //pen->charging = rawPenList[i]["charging"].asInt();
        }
        if (nullptr != callback)
            callback(response.code, response.msg, info);
        else
            delete info;

    });
}

void Ap::startEventObserver()
{

    if(!startNotify){
        startNotify = true;
        needStartSseTimer_.fetchAndStoreAcquire(1);
        SSENotify();
    }
}

void Ap::stopEventObserver()
{
    startNotify = false;
    needStartSseTimer_.storeRelaxed(0);
}

void Ap::SSENotify()
{
    QString url;
    url.append("http://");
    url.append(ip_);
    url.append(":");
    url.append("3000");
    url.append("/api/fe-notify");
    QNetworkRequest httpRequest;
    httpRequest.setUrl(QUrl(url));
    httpRequest.setRawHeader(QByteArray("Accept"), QByteArray("text/event-stream"));
    httpRequest.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    httpRequest.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::CacheLoadControl::AlwaysNetwork);
    m_reply = manager_->get(httpRequest);
    connect(m_reply, &QNetworkReply::readyRead, this, &Ap::sseStreamReceived);
    connect(m_reply, &QNetworkReply::finished, this, &Ap::sseStreamFinished);
    connect(m_reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &Ap::sseStreamError);
    timestamp_ = QDateTime::currentDateTime().toMSecsSinceEpoch();
    if(needStartSseTimer_.testAndSetAcquire(1, 0)) {
        QTimer::singleShot(5 * 1000, this, &Ap::sseStreamTimer);
    }
}

void Ap::sseStreamReceived()
{
    timestamp_  = QDateTime::currentDateTime().toMSecsSinceEpoch();
    if(!startNotify) {
        m_reply->close();
        m_reply->deleteLater();
        m_reply = nullptr;
        return;
    }

    if(m_reply->error() != QNetworkReply::NoError) {
        LOG(INFO)<< "sseStreamReceived error: "<<m_reply->error();
        m_reply->close();
        m_reply->deleteLater();
        m_reply = nullptr;
        return;
    }

    QString  response = QString(m_reply->readAll()).replace("data: ", "");
    sseReconnecting_.testAndSetAcquire(1,0);
    if(0 != m_retries) { //已经恢复连接
        LOG(INFO) << "Ap::sseStreamReceived reconnected.";
        m_retries = 0;
    }

    QStringList list = response.split("\n");
    for(QString &item: list) {
        item = item.simplified();
        //qDebug()<<"item: "<< item;
        if (item.isEmpty()) continue;
        if (item == ":keep-alive") continue;

        if(item.startsWith("{") || item.endsWith("}")) {
            sseStreamBufferList_.append(item);
        } else {
          LOG(INFO)<<"invalid stream: "<<item.toStdString();
        }
    }

    while(!sseStreamBufferList_.empty()) {
        if (sseStreamParse(sseStreamBufferList_[0])) {
            sseStreamBufferList_.removeFirst();
        } else {
            if (sseStreamBufferList_.length() >= 2) {//列表前两个元素合并后能否解析
                response = sseStreamBufferList_[0] + sseStreamBufferList_[1];
                if (sseStreamParse(response)) {
                    qDebug()<<"******COMBINE PARSE SUCCESS ****";
                    sseStreamBufferList_.removeFirst();
                    sseStreamBufferList_.removeFirst();//解析成功去掉表头元素
                }else {//不能解析，也要去掉，没法处理了。
                    LOG(INFO)<<"******COMBINE PARSE FAILURE ****";
                    LOG(INFO)<<sseStreamBufferList_[0].toStdString();
                    sseStreamBufferList_.removeFirst();
                    LOG(INFO)<<sseStreamBufferList_[0].toStdString();
                    sseStreamBufferList_.removeFirst();  
                }
            } else {//列表里只剩一个或0个元素，那么该元素不是一个完整的json,剩余的部分由下次sseStreamReceived获取
                LOG(INFO)<<"COMBINE PARSE: sseStreamBufferList_.size: "<<sseStreamBufferList_.length();
                break;
            }
        }
    }
}

bool Ap::sseStreamParse(QString response)
{
    istringstream iss(response.toStdString());
    CharReaderBuilder rBuilder;
    Value root;
    String err;
    bool ok = false;
    try {
        ok = parseFromStream(rBuilder, iss, &root, &err);
    }catch(Json::Exception e) {
        LOG(INFO)<< "1.sseStreamReceived error: "<<e.what();
    }

    if(!ok) {
        LOG(INFO)<<"parse error: "<<err.c_str() ;
        return ok;
    }

    try {
        const char* type = root["type"].asCString();

        if(strcmp(type, ScanStatusEvent) == 0) {
            Json::Value data = root["data"];
            qDebug()<<"ApInfo.mac="<<this->apInfo_->mac<<", status="<<data["status"].asCString();
            this->apInfo_->status = SCANNING;
            emit APStatusChangeData(*apInfo_);
        }else if(strcmp(type, ConnectionStatusEvent) == 0){
            //这个事件可能返回列表，列表中字段跟返回非数组时，字段结构不一样
            Json::Value data = root["data"];
            if(data.isArray()){
                for(unsigned int i = 0; i < data.size(); ++i) {
                    QString mac = QString(data[i]["mac"].asCString());
                    PenInfo *pen = nullptr;
                    if(this->pairedPenList_.contains(mac)) {
                       pen = this->pairedPenList_[mac];
                    }else{
                       pen = new PenInfo();
                       pen->mac = mac;
                       this->pairedPenList_.insert(mac, pen);
                    }

                    pen->name = QString(data[i]["name"].asCString());
                    pen->connected = data[i]["connected"].asInt();
                    pen->paired = 1;//这种数据的笔，一定是配对的，且发生了重链接
                    emit SSEConnectStatusData(*pen);
                }
            } else {
                QString mac =  QString(data["mac"].asCString());
                int paired = data["inAutoConnectList"].asBool()? 1 : 0;
                PenInfo *pen = nullptr;
                if(paired){
                    if(this->pairedPenList_.contains(mac)){
                       pen = this->pairedPenList_[mac];
                    }else {
                       pen = new PenInfo();
                       pen->mac = mac;
                       this->pairedPenList_.insert(mac, pen);
                    }

                } else {
                    if(this->scanedPenList_.contains(mac)){
                       pen = this->scanedPenList_[mac];
                    }else {
                       pen = new PenInfo();
                       pen->mac = mac;
                       this->scanedPenList_.insert(pen->mac, pen);
                    }
                }
                pen->name = QString(data["name"].asCString());
                pen->connected = strcmp(data["connectionState"].asCString(), "disconnected") == 0? 0 : 1;
                pen->paired = paired;
                emit SSEConnectStatusData(*pen);
            }

        } else if(strcmp(type, ScanDataEvent) == 0){
            Json::Value data = root["data"];
            QList<PenInfo> scanData;
            for(unsigned int i = 0; i < data.size(); ++i) {
                QString mac = QString(data[i]["deviceMac"].asCString());
                PenInfo *pen = nullptr;
                if(this->scanedPenList_.contains(mac)) {
                   pen = this->scanedPenList_[mac];
                }else{
                   pen = new PenInfo();
                   pen->mac = mac;
                   this->scanedPenList_.insert(pen->mac, pen);
                }

                pen->flag = QString(data[i]["flag"].asCString());
                pen->name = QString(data[i]["rawName"].asCString());
                pen->timestamp = data[i]["timestamp"].asUInt64();
                pen->paired = 0;
                pen->connected = 0;

                scanData.append(*pen);
            }
            emit SSEScanStatusData(scanData);
        }
    } catch(Json::Exception e) {
        LOG(INFO)<< "2.sseStreamParse error: "<<e.what() << ",sseStream response:\n "<< response.toStdString();
    }
    return ok;
}

void Ap::sseStreamFinished()
{
    LOG(INFO)<<"sseStreamFinished.";
    restartSseStream();
}

void Ap::sseStreamError(QNetworkReply::NetworkError error)
{
    LOG(INFO)<<"sseStreamError: "<< error;
//    restartSseStream();
}

void Ap::restartSseStream()
{
    timestamp_  = QDateTime::currentDateTime().toMSecsSinceEpoch();
    if(nullptr != m_reply) {
        m_reply->deleteLater();
        m_reply = nullptr;
    }

    if (m_retries < 5 && startNotify) {
        m_retries++;
        apInfo_->status = RECONNECTING;
        sseReconnecting_.testAndSetAcquire(0,1);
        QTimer::singleShot(6*1000,this,&Ap::sseReconnectTimeout); //不能马上去恢复，休眠时系统可能还没有准备好
        LOG(INFO) << "Ap::restartSseStream m_retries=" << m_retries << ",needStartSseTimer_=" << needStartSseTimer_.loadAcquire();
        needStartSseTimer_.fetchAndStoreAcquire(1);
    }else {
        LOG(INFO) << "Unable to reconnect, max retries reached or startNotify="<<startNotify;
        sseReconnecting_.testAndSetAcquire(1,0);
        apInfo_->status = DISCONNECTED;
    }

    if (startNotify){
        emit APStatusChangeData(*apInfo_);
    }
}

void Ap::sseStreamTimer()
{
    int need = needStartSseTimer_.fetchAndStoreAcquire(1);
    LOG(INFO)<<"sseStreamTimer. current.needStartSseTimer_="<<need;
//    if((QDateTime::currentDateTime().toMSecsSinceEpoch() - timestamp_) >= 5 * 1000) {
        //再查下server.ip如果可以调通，则是误报。
//        LOG(INFO)<<"sseStreamTimer timeout, do checkApServerinfo.";
        sseCheckApServerinfo();
//    } else if (true == startNotify) {
//        timestamp_  = QDateTime::currentDateTime().toMSecsSinceEpoch();
//        if(needStartSseTimer_.testAndSetAcquire(1, 0)) {
//            QTimer::singleShot(5 * 1000, this, &Ap::sseStreamTimer);
//        }
//    }
}

void Ap::sseReconnectTimeout()
{
    LOG(INFO) << "Ap::sseReconnectTimeout m_retries=" << m_retries;
    SSENotify();
}

void Ap::sseCheckApServerinfo()
{
    getServerInfo([this](int code, QString, ServerInfo* info) {

        auto networkList = QNetworkInterface::allAddresses();
        QList<QString> localAddrs;
        foreach (auto addr, networkList) {
            if(!addr.isLinkLocal() &&
                    !addr.isLoopback() &&
                    QAbstractSocket::IPv4Protocol == addr.protocol()) {
                LOG(INFO) << "APManager::sseCheckApServerinfo local ip:" << addr.toString().toStdString();
                localAddrs.push_back(addr.toString());
            }
        }

        if (nullptr != info &&
                (info->ip != this->serverInfo_->ip ||
                 !localAddrs.contains(this->serverInfo_->ip )))
        {
            //检测到这个ap的server.ip发生变化，可能被另外一台pc上的管理工具踢掉； 也可能本机网络发生变化了，这可能用重新初始化tqlsdk.
            this->apInfo_->status = CFGCHANGED;
            LOG(INFO) <<"ap.mac="<<apInfo_->mac.toStdString()<< ", CFGCHANGED, old ap.server.ip="
                     << this->serverInfo_->ip.toStdString()<< ", new ap.server.ip="<<info->ip.toStdString() ;
            emit APStatusChangeData(*(this->apInfo_));
        } else if (0< code && code <= QNetworkReply::NetworkError::UnknownServerError) {
            //调用网络错误，应该是断开了，由SSE reply来回复
            int sseConnecting = sseReconnecting_.loadAcquire();
            LOG(INFO)<<"check Ap.server.info network error*****sseReconnecting_=" << sseConnecting;
            if(1 != sseConnecting) {
                LOG(INFO)<<"check Ap.server.info network error*****startNotify=" << startNotify;
                apInfo_->status = DISCONNECTED;
                if (startNotify) {
                    emit APStatusChangeData(*apInfo_);
                }
            }
        } else {
            timestamp_  = QDateTime::currentDateTime().toMSecsSinceEpoch();
            if(needStartSseTimer_.testAndSetAcquire(1, 0)) {
                QTimer::singleShot(5 * 1000, this, &Ap::sseStreamTimer);
            }
        }
        delete info;
    });
}

void Ap::doGet(QString api, std::function<void(JsonResponse)> callback)
{
    QString url;
    url.append("http://");
    url.append(ip_);
    url.append(":");
    url.append("3000");
    url.append(api);
    QNetworkRequest httpRequest;
    httpRequest.setUrl(QUrl(url));
    httpRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    qDebug() << "url====" << url <<endl;
    QNetworkReply* reply = manager_->get(httpRequest);

    connect(reply, &QNetworkReply::finished, [callback,reply](){
        JsonResponse response;
        if(reply->error() == QNetworkReply::NoError) {
            QByteArray bytes = reply->readAll();
            char * data = bytes.data();
            LOG(INFO)<<"GET url: " << reply->url().toString().toStdString()<< "\tcontent: "<< QString(bytes).toStdString()<<endl;

            istringstream iss(data);
            CharReaderBuilder rBuilder;
            Value root;
            String err;
            bool ok = parseFromStream(rBuilder, iss, &root, &err);
            if (ok) {
                response.code = root["code"].asInt();
                response.msg = QString(root["msg"].asCString());
                response.data = root["data"];
                if (nullptr != callback) {
                    callback(response);
                }
            }else {
                if (nullptr != callback) {
                    response.code = 500;
                    response.msg = "parse error:"+ QString(err.c_str());
                    qDebug()<<response.msg<<endl;
                    callback(response);
                }
            }

        } else {
            LOG(INFO)<< "network error: " << reply->error()<<endl;
            if (nullptr != callback) {
                response.code = reply->error();
                response.msg = "network error.";
                callback(response);
            }
        }
        reply->close();
        reply->deleteLater();
    });
}

void Ap::doPost(QString api, QString params, std::function<void(JsonResponse)> callback)
{
    QByteArray* postData =  new QByteArray(params.toStdString().c_str());

    QString url;
    url.append("http://");
    url.append(ip_);
    url.append(":");
    url.append("3000");
    url.append(api);
    QNetworkRequest httpRequest;

    httpRequest.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    httpRequest.setUrl(QUrl(url));
    qDebug() << "url===" << url << endl;
    QNetworkReply* reply = manager_->post(httpRequest, *postData);
    connect(reply, &QNetworkReply::finished, [callback,reply, postData](){
        JsonResponse response;
        if(reply->error() == QNetworkReply::NoError) {
            QByteArray bytes = reply->readAll();
            char * data = bytes.data();
            LOG(INFO) <<"POST url: " << reply->url().toString().toStdString()<< "\tcontent: " << QString(bytes).toStdString()<<endl;

            istringstream iss(data);
            CharReaderBuilder rBuilder;
            Value root;
            String err;
            bool ok = parseFromStream(rBuilder, iss, &root, &err);
            if (ok) {
                response.code = root["code"].asInt();
                response.msg = QString(root["msg"].asCString());
                response.data = root["data"];
                if (nullptr != callback) {
                    callback(response);
                }
            }else {
                if (nullptr != callback) {
                    response.code = reply->error();
                    response.msg = "parse error:" + QString(err.c_str());
                    callback(response);
                }
            }

        } else {
            LOG(INFO)<< "network error: " << reply->error()<<endl;
            if (nullptr != callback) {
                response.code = reply->error();
                response.msg = "network error.";
                callback(response);
            }
        }
        reply->close();
        reply->deleteLater();
        delete postData;
    });
}

