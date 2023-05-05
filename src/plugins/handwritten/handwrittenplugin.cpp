#include "handwrittenplugin.h"

#include "handwrittenwrapper.h"

HandWrittenPlugin::HandWrittenPlugin(QObject *parent)
    : QObject(parent)
{
}

int HandWrittenPlugin::singleRecognize(const QString &userImgPath, int &outLabel, int& contentCode, QString& contentName, int expectLabel)
{
    std::string imgPath = userImgPath.toStdString();
    std::string strName;
    int score = HandWrittenWrapper::instance()->singleRecognize(imgPath,outLabel,contentCode,
                                                           strName,expectLabel);
    contentName = QString::fromStdString(strName);
    return score;
}

