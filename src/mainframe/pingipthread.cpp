#include "pingipthread.h"
//#include "searchipformac.h"

PingIpThread::PingIpThread(QUuid id, QList<QString> ipList):
    isStart(true),
    id(id)
{
    this->mIpList = ipList;
}

PingIpThread::~PingIpThread()
{
    mutex_.lock();
    for(QProcess* p: mProcessList_){
        LOG(INFO)<<"p.kill:"<<p->pid();
        p->kill();
    }
    mutex_.unlock();
    mProcessList_.clear();
    qDebug()<<"~PingIpThread";
}

void PingIpThread::run()
{
    for(int i=0; i< mIpList.length() && isStart; i++){
        QString ip = mIpList.at(i);
        int exitCode;
            //对每个Ip执行ping命令检测其是否在线
        LOG(INFO) << "ping " + ip.toStdString();

        QString strArg = "ping " + ip + " -n 1 -w 1000";
        exitCode = QProcess::execute(strArg);

        if(0 == exitCode)
        {

            QStringList arguments;
            arguments << "/c" << "arp -a " << ip;
            QProcess process;
            mProcessList_.append(&process);
            process.start("cmd.exe", arguments);
            process.waitForStarted();
            process.waitForFinished();
            mutex_.lock();
            mProcessList_.removeOne(&process);
            mutex_.unlock();
            QString dosRet = QString::fromLocal8Bit(process.readAllStandardOutput());


            QString pattern("([0-9A-Fa-f]{2})(-[0-9A-Fa-f]{2}){5}");
            QRegExp rx(pattern);
            int pos = dosRet.indexOf(rx);
            LOG(INFO) << "mac: " + dosRet.toStdString();
            if(pos >= 0 && isStart){
                QString mac = dosRet.mid(pos, 17);
                emit ipForMac(id, false, mac,ip);
            }

        } else {
            LOG(INFO) << "shell ping " + ip.toStdString() + " failed!";
        }
    }

    isStart = false;
    emit ipForMac(id, true,"","");
}

bool PingIpThread::getIsStart()
{
    return isStart;
}

void PingIpThread::stopPing()
{
    isStart = false;
    requestInterruption();
    quit();
    wait();//必须等待结束
}


