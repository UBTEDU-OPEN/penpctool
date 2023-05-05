#include "apmanager.h"
#include "json/json.h"
#include "logHelper.h"
#include <QTimer>
#include <QList>

#include <QDebug>
#include<QtMath>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QTime>
#include <QMetaType>
#include <QNetworkInterface>
#include <QProcess>

#define AddAp  "addAp"
#define PenPair "penPair"
#define PenUnpair "penUnpair"
#define GetPenList "getPenList"

using namespace Json;

APManager::APManager(QObject *parent) : QObject(parent)
  , apScanTimer_(nullptr)
{
    qRegisterMetaType<PenInfo>("PenInfo");
    qRegisterMetaType<APInfo>("APInfo");
    qRegisterMetaType<QList<PenInfo>>("QList<PenInfo>");
}

APManager::~APManager(){
    qDeleteAll(apList_);
    apList_.clear();
    LOG(INFO) << "~APManager";
}

APManager* APManager::getInstance()
{
    static APManager* instance_ = new APManager;
    return instance_;
}

void APManager::handlePenPower(QString mac, int Battery, int Charging)
{
    for(Ap *ap: apList_) {
        ap->updatePenPower(mac, Battery, Charging);
    }
}

void APManager::handlePenFirmwareVersion(QString mac, QString FN)
{
    for(Ap *ap: apList_) {
        ap->updatePenFirmwareVersion(mac, FN);
    }
}

void APManager::saveMacIp(QString mac, QString ip)
{
    LOG(INFO) << "APManager::saveMacIp mac=" << mac.toStdString() << ",ip=" << ip.toStdString();
    QMutexLocker lock(&macIpMutex_);
    macIpMap_.insert(mac,ip);
}

void APManager::removeMac(QString mac)
{
    LOG(INFO) << "APManager::removeMac mac=" << mac.toStdString();
    QMutexLocker lock(&macIpMutex_);
    macIpMap_.remove(mac);
}

QString APManager::getIpOfMac(QString mac)
{
    LOG(INFO) << "APManager::getIpOfMac mac=" << mac.toStdString();
    QMutexLocker lock(&macIpMutex_);
    if(macIpMap_.contains(mac)) {
        return macIpMap_[mac];
    }
    return "";
}

QStringList APManager::getLocalIps()
{
    auto networkList = QNetworkInterface::allAddresses();
    QStringList validAddrList;
    foreach (auto addr, networkList) {
        if(!addr.isLinkLocal() &&
                !addr.isLoopback() &&
                QAbstractSocket::IPv4Protocol == addr.protocol()) {
            LOG(INFO) << "APManager::getLocalIps local ip:" << addr.toString().toStdString();
            validAddrList.push_back(addr.toString());
        }
    }
    return validAddrList;
}

bool APManager::ApHasSetuped(QString ip){
    QList<Ap*>::const_iterator iter = apList_.begin();
    for(; iter != apList_.end(); ++iter){
        if ((*iter)->getAPInfo().ip == ip && (*iter)->isSetuped()) {
            return true;
        }
    }
    return false;
}

void APManager::setupAp(MacAndIp macIp, QString msgId, QString localAddr)
{
    ServerInfo info;
    info.ip = localAddr; //SearchIpForMac::get_instance()->getHostIpAddress();
    info.port = port;
    info.type = "ap0";
    info.proto = "tcp";
    Ap* ap = new Ap(macIp.ip, macIp.mac);
    ap->setup(info, [this, ap, msgId, macIp](int code , QString msg) {
        {
            QMutexLocker lock(&pendingMacsMutex_);
            pendingMacs_.remove(macIp.mac);
        }
        if (code == 0) {
            this->apList_.append(ap);
            emit sendBuiltMsg(buildAddApResponse(msgId, 0, "success", macIp.mac));
            Config::saveBufferIp(macIp.mac,macIp.ip);
            connect(ap, &Ap::APStatusChangeData, this, &APManager::handleAPStatusChangeData);
            connect(ap, &Ap::SSEConnectStatusData, this, &APManager::SSEConnectStatusData);
            connect(ap, &Ap::PenPowerChanged, this, &APManager::PenPowerChanged);
            connect(ap, &Ap::SSEScanStatusData, this, &APManager::SSEScanStatusData);
            connect(ap, &Ap::noFN,this, &APManager::noFN);
        }else{
            LOG(WARNING) <<"addApp error:"<<msg.toStdString();
            removeMac(macIp.mac);
            emit sendBuiltMsg(buildAddApResponse(msgId, code, msg.toStdString(), macIp.mac));
        }
    });
}

void APManager::onAddAp(QString msgId, QList<QString> ipOrMacList)
{
    qDebug()<<"ipOrMacList.size = "<<ipOrMacList.length();
    if (ipOrMacList.isEmpty()) {
        emit sendBuiltMsg(buildAddApResponse(msgId, 400, "paramter error, empty", ""));
        return;
    }
    qDeleteAll(apList_);
    apList_.clear();

    QMap<QString,QString> macIps = getMacIp();
    //判断预扫描的IP是否都有效，如果不是，则网络环境有变化，需要重新扫描
    bool allIpIsValid = true;
    for(auto it = macIps.cbegin(); it != macIps.cend(); ++it) {
        if(!isApIpValid(it.key(),it.value())) {
            allIpIsValid = false;
            break;
        }
    }

    bool allMacFound = true;
    for(auto mac : ipOrMacList) {
        if(!macIps.contains(mac)) {
            allMacFound = false;
            break;
        }
    }

    if(!allIpIsValid || !allMacFound) {
        {
            QMutexLocker lock(&pendingMacsMutex_);
            pendingMacs_ = ipOrMacList.toSet();
        }
        Config::clearBufferIp();
        scanAvailableAps(true,msgId);
    } else {
        QMap<QString,QString> usefulMacIps;
        for (auto mac : ipOrMacList) {
            QString ip = macIps[mac];
            usefulMacIps.insert(mac,ip);
            LOG(INFO)<< "ip===" << mac.toStdString() << ";mac===" << ip.toStdString();
            if(ApHasSetuped(ip)) {
                emit sendBuiltMsg(buildAddApResponse(msgId, 0, "success", mac));
            } else {
                MacAndIp macIp;
                macIp.mac = mac;
                macIp.ip = ip;

                QString netSegment = ip.mid(0,ip.lastIndexOf(".")+1);

                LOG(INFO) << "APManager::scanAvailableAps netSegment:" << netSegment.toStdString();

                auto networkList = QNetworkInterface::allAddresses();
                QString localAddr;
                foreach (auto addr, networkList) {
                    if(!addr.isLinkLocal() &&
                            !addr.isLoopback() &&
                            QAbstractSocket::IPv4Protocol == addr.protocol()) {
                        LOG(INFO) << "APManager::scanAvailableAps local ip:" << addr.toString().toStdString();
                        QString tempAddr = addr.toString();
                        if(tempAddr.contains(netSegment)) {
                            localAddr = tempAddr;
                            break;
                        }
                    }
                }
                LOG(INFO) << "APManager::scanAvailableAps same segment local ip:" << localAddr.toStdString();

                if(!localAddr.isEmpty()) {
                    setupAp(macIp, msgId,localAddr);
                }

            }
        }
        Config::saveBufferIp(usefulMacIps);
    }
}

void APManager::onPenPair(QString msgId, QList<QString> macList)
{
    if (macList.size() == 0) {
        emit sendNoDataResponse(msgId,PenPair,400,"mac list is empty");
        return;
    }

    QList<bool> *overList = new QList<bool>;
    double apLen = apList_.length();
    int average = qCeil(macList.size() / apLen);
    if (average > 0) {
        QList<QList<QString>> averageList;
        QVector<QString> array = macList.toVector();

        //切片
        for (QVector<QString>::iterator iter = array.begin(); iter < array.end(); iter = iter + average) {
            if (iter + average < array.end()) {
                QList<QString> cut1(iter, iter + average);
                averageList.append(cut1);
            }else {
                QVector<QString>::iterator end = array.end();
                QList<QString> cut2(iter, end);
                averageList.append(cut2);
            }
        }

        //配对
        qDebug()<<"apList.size="<<apList_.length()<<", averageList.size="<<averageList.length() <<endl;
        for (int i = 0; i < apList_.length() && i < averageList.length(); i++) {
            overList->append(false);
            apList_.at(i)->pairPens(averageList.at(i), [overList](int , QString ){
                overList->removeFirst();
            });
        }
    }
    else {// 注意，如果支持Ap > 4，下面分支需要优化。
        QList<Ap*>::const_iterator iter = apList_.begin();
         Ap* minPairedAp = nullptr;
        for(; iter != apList_.end(); ++iter){
            if (minPairedAp == nullptr) {
                minPairedAp = *iter;
            }
            else {
                if((*iter)->getCurrentPairedPenMacList().length() < minPairedAp->getCurrentPairedPenMacList().length())
                {
                    minPairedAp = *iter;
                }
            }
        }
        if(minPairedAp != nullptr){
            overList->append(false);
            minPairedAp->pairPens(macList, [overList](int , QString ){
                overList->removeFirst();
            });
        }
    }

    QTimer *timer = new QTimer;
    connect(timer, &QTimer::timeout, [this, msgId, overList, timer](){
        if(overList->isEmpty()){
            timer->stop();
            timer->deleteLater();
            delete overList;
            emit sendNoDataResponse(msgId,PenPair,0,"success");
        }
    });
    timer->start(100);
}

void APManager::onPenUnPair(QString msgId, QList<QString> macList)
{
    QList<bool> *overList = new QList<bool>;

    if(macList.empty()) {
        //如果list为null, 每个AP解除所有配对的笔
        QList<Ap*>::const_iterator iter = apList_.cbegin();
        for(; iter != apList_.cend(); ++iter){
            overList->append(false);
            //do all unpair
            (*iter)->unpairPens((*iter)->getCurrentPairedPenMacList(), [overList](int , QString ){
                overList->removeFirst();
            });
        }
    }
    else {
        QList<Ap*>::const_iterator iter = apList_.cbegin();
        for(; iter != apList_.cend(); ++iter){
            overList->append(false);
            (*iter)->unpairPens(macList, [overList](int , QString ){
                overList->removeFirst();
            });
        }

    }

    QTimer *timer = new QTimer;
    connect(timer, &QTimer::timeout, [this, msgId, overList, timer](){
        if(overList->isEmpty()){
            timer->stop();
            timer->deleteLater();
            delete overList;
            emit sendNoDataResponse(msgId,PenUnpair,0,"success");
        }
    });
    timer->start(100);
}

void APManager::onGetPenList(QString msgId, QList<QString> macList)
{
    QMap<QString,PenInfo> ret;
    if (macList.size() == 0 ) {
        // all pens, and
        QList<Ap*>::const_iterator iter = apList_.cbegin();
        for(; iter != apList_.cend(); ++iter){
             QList<PenInfo> l = (*iter)->getAllPens();
             QList<PenInfo>::const_iterator it = l.cbegin();
             for (; it != l.cend(); ++it){
                QString mac = (*it).mac;
                if (ret.contains(mac)) {
                    if((*it).connected == 1) {
                        ret[mac].connected = 1;
                        ret[mac].paired = 1;
                    }
                } else {
                    ret.insert(mac, *it);
                }
             }
        }

    }else {
        // only macList
        QList<Ap*>::const_iterator iter = apList_.cbegin();
        for(; iter != apList_.cend(); ++iter){
             QList<PenInfo> l = (*iter)->getAllPens();
             QList<PenInfo>::const_iterator it = l.cbegin();
             for (; it != l.cend(); ++it){
                QString mac = (*it).mac;
                if (macList.contains(mac)) {
                    if (ret.contains(mac)) {
                        if((*it).connected == 1) {
                            ret[mac].connected = 1;
                            ret[mac].paired = 1;
                        }
                    } else {
                        ret.insert(mac, *it);
                    }
                }
             }
        }
    }

    QList<PenInfo> gobalList = ret.values();
    emit sendBuiltMsg(buildGetPenListResponse(msgId, 0, "success", gobalList));
}

void APManager::handleAPStatusChangeData(APInfo apInfo)
{
    qDebug() << "handleAPStatusChangeData====" << apInfo.mac;
    if (apInfo.status == CFGCHANGED) {
        Ap* temp = nullptr;
        for(Ap* ap: apList_){
            if(ap->getAPInfo().mac == apInfo.mac) {
                temp = ap;
            }
        }
        if(nullptr != temp){
            apList_.removeOne(temp);
            temp->deleteLater();
        }
        LOG(INFO)<<"Ap.mac="<<apInfo.mac.toStdString()<<", Ap.status="<<apInfo.status.toStdString();
        apInfo.status = DISCONNECTED;//直接认定为断开，因为AP已删除了

    }else if (apInfo.status == DISCONNECTED) {
        Ap* temp = nullptr;
        for(Ap* ap: apList_){
            if(ap->getAPInfo().mac == apInfo.mac) {
                temp = ap;
            }
        }
        if(nullptr != temp){
            apList_.removeOne(temp);
            temp->deleteLater();
            temp = nullptr;
        }
        LOG(INFO)<<"Ap.mac="<<apInfo.mac.toStdString()<<", Ap.status="<<apInfo.status.toStdString();

    }else if (apInfo.status == RECONNECTING) {
        LOG(INFO)<<"Ap.mac="<<apInfo.mac.toStdString()<<", Ap.status="<<apInfo.status.toStdString();
    }

    emit ApStatusData(apInfo);
}

void APManager::onScanApTimeout()
{
    apScanTimer_->deleteLater();
    apScanTimer_ = nullptr;
    QSet<QString> pendingMacs;
    {
        QMutexLocker lock(&pendingMacsMutex_);
        pendingMacs = pendingMacs_;
    }
    for (QString mac : pendingMacs) {
        emit sendBuiltMsg(buildAddApResponse(tempAddApMsgId_, -1, " Unable to find IP based on MAC", mac));
    }
}

APInfo APManager::getAPInfo(const QString &apid)
{
    QList<Ap*>::const_iterator iter = apList_.cbegin();
    for (; iter != apList_.cend(); ++iter) {
        if((*iter)->ip_ == apid && (*iter)->isSetuped()) {
            return (*iter)->getAPInfo();
        }else if ((*iter)->mac_ == apid && (*iter)->isSetuped()) {
            return (*iter)->getAPInfo();
        }
    }
    return APInfo();
}

QList<APInfo> APManager::getApList()
{
    QList<APInfo> infos;
    for(auto ap : apList_) {
        infos.push_back(ap->getAPInfo());
    }
    return infos;
}

ServerInfo APManager::getServerInfo(const QString& apid)
{
    QList<Ap*>::const_iterator iter = apList_.cbegin();
    for (; iter != apList_.cend(); ++iter) {
        if((*iter)->ip_ == apid && (*iter)->isSetuped()) {
            return (*iter)->getServerInfo();
        }else if ((*iter)->mac_ == apid && (*iter)->isSetuped()) {
            return (*iter)->getServerInfo();
        }
    }
    return ServerInfo();
}

void APManager::startScan()
{
    QList<Ap*>::const_iterator iter = apList_.cbegin();
    for(; apList_.cend() != iter; ++iter){
        (*iter)->startScan([](int, QString msg){
            qDebug()<<"startScan:"<<msg<<endl;
        });
    }
}

void APManager::stopScan()
{
    QList<Ap*>::const_iterator iter = apList_.cbegin();
    for(; apList_.cend() != iter; ++iter){
        (*iter)->stopScan([](int, QString msg){
            qDebug()<<"stopScan:"<<msg<<endl;
        });
    }
}

QString APManager::buildGetPenListResponse(QString id, int code, std::string msg, QList<PenInfo>& pens)
{
    StreamWriterBuilder wBuilder;
    Value root;
    root["id"] = id.toStdString();
    root["command"] = GetPenList;
    Value body;
    body["code"] = code;
    body["msg"] = msg;

    Value data;

    Value penList;
    Json::Value penInfo;

    QList<PenInfo>::const_iterator iter = pens.cbegin();
    for (; iter != pens.cend(); ++iter)
    {
        penInfo["mac"] = (*iter).mac.toStdString();
        penInfo["name"] = (*iter).name.toStdString();
        penInfo["connected"] = (*iter).connected;
        penInfo["paired"] = (*iter).paired;
        penList.append(penInfo);
    }

    data["penList"] = penList;
    body["data"] = data;
    root["body"] = body;
    return QString(writeString(wBuilder, root).c_str());
}

QString APManager::buildAddApResponse(QString id, int code, std::string msg, QString mac)
{
    StreamWriterBuilder wBuilder;
    Value root;
    root["id"] = id.toStdString();
    root["command"] = AddAp;
    Value body;
    body["code"] = code;
    body["msg"] = msg;

    Value data;
    data["mac"] = mac.toStdString();
    body["data"] = data;
    root["body"] = body;
    return QString(writeString(wBuilder, root).c_str());
}

void APManager::scanAvailableAps(bool emitMsg, QString msgId)
{
    {
        QMutexLocker lock(&macIpMutex_);
        macIpMap_.clear();
    }

    if(emitMsg) {
        tempAddApMsgId_ = msgId;
        apScanTimer_ = new QTimer;
        apScanTimer_->setSingleShot(true);
        connect(apScanTimer_,&QTimer::timeout,this,&APManager::onScanApTimeout);
        apScanTimer_->start(60*1000);
    }

    auto networkList = QNetworkInterface::allAddresses();
    QList<QHostAddress> validAddrList;
    foreach (auto addr, networkList) {
        if(!addr.isLinkLocal() &&
                !addr.isLoopback() &&
                QAbstractSocket::IPv4Protocol == addr.protocol()) {
            LOG(INFO) << "APManager::scanAvailableAps local ip:" << addr.toString().toStdString();
            validAddrList.push_back(addr);
        }
    }

    foreach (auto hostIp, validAddrList) {
        QString hostIpStr = hostIp.toString();
        auto pos = hostIpStr.lastIndexOf('.');
        auto preStr = hostIpStr.mid(0,pos+1);
        for(int i = 1; i < 255; ++i) {
            auto newIp = preStr + QString::number(i);
            if(newIp == hostIpStr) {
                continue;
            }
            QString url;
            url.append("http://");
            url.append(newIp);
            url.append(":3000/api/report-way");
            QNetworkRequest httpRequest;
            httpRequest.setUrl(QUrl(url));
            httpRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
            QNetworkReply* reply1 = manager_.get(httpRequest);
            QObject::connect(reply1, &QNetworkReply::finished, [this,reply1,newIp,emitMsg,msgId,hostIpStr](){
                if(reply1->error() == QNetworkReply::NoError) {
                    QByteArray bytes = reply1->readAll();
                    QJsonObject obj = QJsonDocument::fromJson(bytes).object();
                    int code = obj["code"].toInt();
                    if(0 == code) {
                        QString url;
                        url.append("http://");
                        url.append(newIp);
                        url.append(":3000/api/app-info");
                        QNetworkRequest httpRequest;
                        httpRequest.setUrl(QUrl(url));
                        httpRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
                        QNetworkReply* reply2 = manager_.get(httpRequest);
                        QObject::connect(reply2, &QNetworkReply::finished, [this,reply2,newIp,emitMsg,msgId,hostIpStr](){
                            if(reply2->error() == QNetworkReply::NoError) {
                                QByteArray bytes = reply2->readAll();
                                QJsonObject obj = QJsonDocument::fromJson(bytes).object();
                                int code = obj["code"].toInt();
                                if(0 == code) {
                                    QJsonObject data = obj["data"].toObject();
                                    QString mac = data["mac"].toString();
                                    saveMacIp(mac.remove(':'),newIp);
                                    MacAndIp macIp;
                                    macIp.mac = mac;
                                    macIp.ip = newIp;
                                    if(emitMsg) {
                                        bool pendingMac = false;
                                        {
                                            QMutexLocker lock(&pendingMacsMutex_);
                                            pendingMac = pendingMacs_.contains(mac);
                                        }
                                        if(pendingMac) {
                                            setupAp(macIp,msgId,hostIpStr);
                                        }
                                    }
                                }
                            }
                            reply2->close();
                            reply2->deleteLater();
                        });
                    }
                }
                reply1->close();
                reply1->deleteLater();
            });
        }
    }
}

bool APManager::isApIpValid(QString mac, QString ip)
{
    QString strArg = "ping " + ip + " -n 1 -w 1000";
    int exitCode = QProcess::execute(strArg);
    if(0==exitCode){
        LOG(INFO) << "getBufferIp===mac"
                  << mac.toStdString() << ";ip="
                  << ip.toStdString() << ";exitCode="
                  << exitCode;


        QStringList arguments;
        arguments << "/c" << "arp -a " << ip;
        QProcess process;
        process.start("cmd.exe", arguments);
        process.waitForStarted();
        process.waitForFinished();
        QString dosRet = QString::fromLocal8Bit(process.readAllStandardOutput());

        QString pattern("([0-9A-Fa-f]{2})(-[0-9A-Fa-f]{2}){5}");
        QRegExp rx(pattern);
        int pos = dosRet.indexOf(rx);
        if(pos >= 0){
            QString arpMac = dosRet.mid(pos, 17);
            arpMac.remove(QChar('-'));
            arpMac = mac.toUpper();
            if(arpMac == mac){
                return true;
            }
        }
    }
    return true;
}

QMap<QString, QString> APManager::getMacIp()
{
    QMutexLocker lock(&macIpMutex_);
    return macIpMap_;
}
