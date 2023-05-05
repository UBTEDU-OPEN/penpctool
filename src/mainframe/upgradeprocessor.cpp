#include "upgradeprocessor.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonValue>

#include "projectconst.h"
#include "config.h"
#include "moduleinfo.h"

UpgradeProcessor *UpgradeProcessor::instance()
{
    static UpgradeProcessor* inst = new UpgradeProcessor;
    return inst;
}

////[{"moduleName":"main","versionName":"1.1.0.1","isIncremental":false,
/// "packageUrl":"https://ubtrobot-new.oss-cn-shenzhen.aliyuncs.com/upgrade-test/2021/12/15/1639551774523/aipen_pc1.0.4.31.zip",
/// "packageMd5":"0842c5cbb834a5b03ca6a9adbff311b4","packageSize":129657040,"isForced":true,
/// "releaseNote":"111111","releaseTime":1639553450}]
void UpgradeProcessor::OnHandleUpgradeInfo(int result, const QJsonArray &js1)
{
    qDebug() << result << js1;
    if(0 == result && js1.size() > 0) {
        QJsonObject module = js1[0].toObject();
        QString versionName = module["versionName"].toString();
        QString packageUrl = module["packageUrl"].toString();
        QString packageMd5 = module["packageMd5"].toString();
        qint64 packageSize = module["packageSize"].toDouble();
        QString releaseNote = module["releaseNote"].toString();
        QString moduleName = module["moduleName"].toString();

        if(moduleName == "main") {
            bool isForced = module["isForced"].toBool();

            int majorVer = versionName.mid(0,versionName.indexOf('.')).toInt();
            int localMajor = Config::localVersion().mid(0,Config::localVersion().indexOf('.')).toInt();
            if(majorVer != localMajor) { //如果大版本升级必然强制升级
                isForced = true;
            }

            emit newVersion(isForced,packageMd5,packageUrl,releaseNote,versionName,packageSize);
        } else if(moduleName == "char_evaluation"){
            emit newCharEvaluation(packageMd5,packageUrl,releaseNote,versionName,packageSize);
        } else if(moduleName == "hand_written") {
            emit newHandWritten(packageMd5,packageUrl,releaseNote,versionName,packageSize);
        } else if(moduleName == "aipen_server") {
            emit newAipenServer(packageMd5,packageUrl,releaseNote,versionName,packageSize);
        }
    }
}

UpgradeProcessor::UpgradeProcessor(QObject *parent) : QObject(parent)
{

}
