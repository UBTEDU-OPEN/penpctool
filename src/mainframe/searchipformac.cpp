#include "searchipformac.h"
#include "logHelper.h"
#include <QUuid>

SearchIpForMac::SearchIpForMac(QObject *parent): QObject(parent),
    macsSize(0),
    isStartSearch(false),
    currentId(QUuid::createUuid())
{

}

void SearchIpForMac::stopAllThread(QUuid id)
{
    //LOG(INFO)<< "stopAllThread====";

    if(pingIpThread1[id] != nullptr){
        pingIpThread1[id]->stopPing();
    }
    if(pingIpThread2[id] != nullptr){
       pingIpThread2[id]->stopPing();
    }
    if(pingIpThread3[id] != nullptr){
        pingIpThread3[id]->stopPing();
    }
}

void SearchIpForMac::deleteThread(QUuid id)
{
    //LOG(INFO) << "======deleteThread======";

    if(pingIpThread1[id] != nullptr){
        pingIpThread1[id]->deleteLater();
        pingIpThread1[id] = nullptr;
    }
    if(pingIpThread2[id] != nullptr){
        pingIpThread2[id]->deleteLater();
        pingIpThread2[id] = nullptr;
    }
    if(pingIpThread3[id] != nullptr){
        pingIpThread3[id]->deleteLater();
        pingIpThread3[id] = nullptr;
    }
    isStartSearch = false;
}

void SearchIpForMac::getIpForMac(QUuid id, bool isFinished, QString mac, QString ip)
{
    if(!mac.isEmpty() && !ip.isEmpty() && !isFinished){
        mac.remove(QChar('-'));
        mac = mac.toUpper();
        LOG(INFO) << "mac===="<< mac.toStdString() << ";ip====" << ip.toStdString();
        mutex.lock();
        for (int i = 0; i < mMacs.length(); ++i) {
            QString m_mac = mMacs.at(i);
            if(m_mac == mac){
                MacAndIp* macAndIp = new MacAndIp();
                macAndIp->ip = ip;
                macAndIp->mac = mac;
                macList.push_back(macAndIp);
            }
        }
        if(macList.length() == macsSize){
            //查找ip结束，停止所有线程
            stopAllThread(id);
        }
        mutex.unlock();
    }else {
        //qDebug() << "======1111111======";
        std::function<void(QList<MacAndIp*>)> temp = nullptr;
        mutex.lock();
        if((pingIpThread1[id] == nullptr || (pingIpThread1[id] != nullptr && !pingIpThread1[id]->getIsStart())) &&
           (pingIpThread2[id] == nullptr || (pingIpThread2[id] != nullptr && !pingIpThread2[id]->getIsStart())) &&
           (pingIpThread3[id] == nullptr || (pingIpThread3[id] != nullptr && !pingIpThread3[id]->getIsStart()))){
            temp = searchResultcallbacks.take(id);
            stopAllThread(id);
            deleteThread(id);
        }
        mutex.unlock();

        if (nullptr != temp) {
            //qDebug() << "======22222======";
            temp(this->macList);
        }
    }

}

SearchIpForMac::~SearchIpForMac()
{
    qDebug()<< "~SearchIpForMac";

}

SearchIpForMac* SearchIpForMac::get_instance()
{
    static SearchIpForMac* instance = new SearchIpForMac;
    return instance;
}

void SearchIpForMac::startScarchIp(QList<QString> macs, std::function<void(QList<MacAndIp*>)> callBack)
{
    if (macs.length() == 0) {
        callBack(QList<MacAndIp*>());
        return;
    }

    QString ip = getHostIpAddress();
    this->macList.clear();
    if(!ip.isNull() && !ip.isEmpty() && !isStartSearch){
        isStartSearch = true;
        this->mMacs = macs;
        this->macsSize = macs.size();
        getBufferIp(macs);
        if (this->mMacs.size() == 0) {
            //cache
            callBack(this->macList);
            isStartSearch = false;
            return;
        }

        QUuid id = QUuid::createUuid();
        this->currentId = id;
        this->searchResultcallbacks.insert(id, callBack);

        int lastIndex = ip.lastIndexOf(".");
        QString lastIp = ip.mid(lastIndex+1,ip.length());
        int intIp = lastIp.toInt();

        QString preIp = ip.mid(0,lastIndex+1);

        QList<QString> ipList1;
        QList<QString> ipList2;
        QList<QString> ipList3;
        for(int i=0; i<255; i++){
            if(i != intIp){
                ip = preIp + QString::number(i);
                if(i<=84){
                    ipList1.append(ip);
                }else if(i>84 && i<=168){
                    ipList2.append(ip);
                }else {
                    ipList3.append(ip);
                }
            }
        }

        pingIpThread1[id] = new PingIpThread(id, ipList1);
        pingIpThread1[id]->start();
        pingIpThread2[id] = new PingIpThread(id, ipList2);
        pingIpThread2[id]->start();
        pingIpThread3[id] = new PingIpThread(id, ipList3);
        pingIpThread3[id]->start();

        connect(pingIpThread1[id], &PingIpThread::ipForMac, this, &SearchIpForMac::getIpForMac);
        connect(pingIpThread2[id], &PingIpThread::ipForMac, this,  &SearchIpForMac::getIpForMac);
        connect(pingIpThread3[id], &PingIpThread::ipForMac, this,  &SearchIpForMac::getIpForMac);



    } else {
        qDebug() << "get ip fail" << endl;
        if(callBack != nullptr){
            callBack(this->macList);
        }
    }
}

void SearchIpForMac::stopScarchIp()
{
    mutex.lock();
    stopAllThread(currentId);
    deleteThread(currentId);
    qDeleteAll(macList);
    //qDebug()<<"1111111111111111111111.test";
    macList.clear();
    mMacs.clear();
    mutex.unlock();
    //qDebug()<<"22222222222222.test";
}

QString SearchIpForMac::getHostIpAddress()
{
    QString strIpAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // 获取第一个本主机的IPv4地址
    int nListSize = ipAddressesList.size();
    for (int i = 0; i < nListSize; ++i)
    {
           if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
               ipAddressesList.at(i).toIPv4Address()) {
               strIpAddress = ipAddressesList.at(i).toString();
               break;
           }
     }
     // 如果没有找到，则以本地IP地址为IP
     if (strIpAddress.isEmpty())
        strIpAddress = QHostAddress(QHostAddress::LocalHost).toString();
     return strIpAddress;
}

QList<MacAndIp *> SearchIpForMac::getSearchAp()
{
    return macList;
}

void SearchIpForMac::getBufferIp(QList<QString> macs)
{
    for(int i=0; i<macs.size();i++){
        QString mac = macs.at(i);
        QString ip = Config::getBufferIp(mac);
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
                    MacAndIp* macIp = new MacAndIp();
                    macIp->ip = ip;
                    macIp->mac = mac;
                    this->macList.push_back(macIp);
                    this->mMacs.removeAll(mac);
                }
            }

        }
    }
}


