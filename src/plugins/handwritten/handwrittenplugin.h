#ifndef HANDWRITTENREC_H
#define HANDWRITTENREC_H

#include <QObject>

#include "handwritteninterface.h"

class HandWrittenPlugin : public QObject, public HandWrittenInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.ubtrobot.HandWrittenPlugin" FILE "handwritten.json")
    Q_INTERFACES(HandWrittenInterface)

public:
    explicit HandWrittenPlugin(QObject *parent = nullptr);

    int singleRecognize(const QString& userImgPath,int &outLabel, int& contentCode, QString& contentName, int expectLabel);
};

#endif // HANDWRITTENREC_H
