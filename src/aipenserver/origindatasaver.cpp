#include "origindatasaver.h"
#include "logHelper.h"

#include <QDebug>
#include <QThread>

#include "config.h"

OriginDataSaver::OriginDataSaver(QObject *parent) : QObject(parent)
{

}

void OriginDataSaver::releaseFile()
{
    for(auto filePtr : originFile) {
        filePtr->close();
        filePtr->deleteLater();
    }
}

void OriginDataSaver::onOriginData(QString macAddr, char* pBuffer, int Bufferlength)
{
    macAddr.remove(':');
    if(!originFile.contains(macAddr)) {
        QString filePath = Config::getTempPath() + macAddr + ".txt";
        QFile* dataFile = new QFile(filePath);
        bool ret = dataFile->open(QFile::WriteOnly | QFile::Text);
        LOG(INFO) << "OriginDataSaver::onOriginData create file " << filePath.toStdString() + ",ret=" << ret;
        if(ret) {
            originFile.insert(macAddr,dataFile);
        }
    }

    if(originFile.contains(macAddr)) {
        int step = 10;
        int j = Bufferlength / step;
        for(int i = 0; i < j; ++i) {
            QByteArray data(pBuffer+i*step,step);
            originFile[macAddr]->write(data.toHex(' ').toUpper().append('\n'));
            originFile[macAddr]->flush();
        }
        int remain = Bufferlength % step;
        if(0 != remain && j > 0) {
            LOG(WARNING) << "TQLSDKAdapter::onReceivePenOriginDot not multiples of 10," << macAddr.toStdString();
            QByteArray data(pBuffer+ j*step,remain);
            originFile[macAddr]->write(data.toHex(' ').toUpper().append('\n'));
            originFile[macAddr]->flush();
        }
    }

    delete [] pBuffer;
}
