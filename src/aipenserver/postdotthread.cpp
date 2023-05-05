#include "PostDotThread.h"



#include <QFile>
#include <QDebug>

#include "logHelper.h"
#include "tqlsdkadapter.h"
#include "config.h"


PostDotThread::PostDotThread(QObject* parent)
    : QThread(parent)
{
}

PostDotThread::~PostDotThread()
{
    qDebug()<<"~PostDotThread";
}

void PostDotThread::stopRunning()
{
    running = false;
}


void PostDotThread::run()
{
    qDebug() << "post dot start, thread id=" << currentThreadId();
    while(!pens.empty() && running) {
        for(auto file = pens.cbegin(); file != pens.cend(); ++file) {
#ifdef SHOW_TEST_BTN
            QString fileKey = file.key();
            QString tempKey;
            for(int i = 0; i < fileKey.size(); ++i) {
                if(i != 0 && i%2 == 0) {
                    tempKey.append(':');
                }
                tempKey.append(fileKey[i]);
            }
            TQLSDKAdapter::pMac = tempKey.toStdString();
            char testMac[20]{0};
            memcpy_s(testMac,20,TQLSDKAdapter::pMac.c_str(), TQLSDKAdapter::pMac.size());
            bool firstLine = true;
#endif
            while(!file.value()->atEnd()) {
                QByteArray line = file.value()->readLine();
                if(line.size() != 30) {
                    LOG(INFO) << "invalid data:" << line.toStdString() << "," << file.key().toStdString();
                    continue;
                }
                QString lineData(line.mid(0,29));
                QStringList dataList = lineData.split(' ');
                LOG(INFO) << "post data..." << lineData.toStdString() << "," << file.key().toStdString();
                unsigned char pData[10]{0};
                bool ok = false;
                for (size_t i = 0; i < dataList.size(); i++) {
                    pData[i] = dataList[i].toInt(&ok, 16);
                }
//                memcpy_s(pData,20,lineData.toStdString().c_str(),20);
                TQLSDKAdapter::getInstance()->onOpenReceiveDot(pData,testMac,false);

                if(firstLine) {
                    firstLine = false;
                    int speed = Config::testWriteSpeed();
                    LOG(INFO) << testMac << " will set speed to " << speed;
                    ::setWriteSpeed(testMac,speed);
                }
            }
            sleep(5);
        }

        for(auto pen : pens) {
            pen->close();
            delete pen;
        }
        pens.clear();
    }
    LOG(INFO) << "post dot finished";
}

