#include "moduleinfo.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include "logHelper.h"

ModuleInfo::ModuleInfo()
{

}

QString ModuleInfo::getVersion(ModuleType moduleType)
{
    QString fileName;
    switch (moduleType) {
    case ModuleType::kCharEvaluation:
        fileName = "charevaluation.json";
        break;
    case ModuleType::kHandWritten:
        fileName = "handwritten.json";
        break;
    case ModuleType::kAipenServer:
        fileName = "aipenserver.json";
        break;
    default:
        break;
    }
    LOG(INFO) << "ModuleInfo::getVersion fileName=" << fileName.toStdString();
    QString ver;
    if(!fileName.isEmpty()) {
        QFile moduleFile(fileName);
        if(moduleFile.open(QFile::ReadOnly)) {
            auto obj = QJsonDocument::fromJson(moduleFile.readAll()).object();
            ver = obj.value("version").toString();
            moduleFile.close();
        }
    }
    return ver;
}

int ModuleInfo::compareVersion(QString oldVersion, QString newVersion)
{
    QStringList oldVers = oldVersion.split('.');
    QStringList newVers = newVersion.split('.');

    for(int i = 0; i < 4; ++i) {
        int oldVer = oldVers[i].toInt();
        int newVer = newVers[i].toInt();
        if(newVer > oldVer) {
            return 1;
        } else if(newVer < oldVer) {
            return -1;
        }
    }

    return 0;
}
