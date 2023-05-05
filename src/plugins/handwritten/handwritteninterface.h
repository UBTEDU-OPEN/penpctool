#ifndef HANDWRITTENINTERFACE_H
#define HANDWRITTENINTERFACE_H

#include <QString>
#include <QObject>

class HandWrittenInterface
{
public:
    virtual ~HandWrittenInterface() = default;

    virtual int singleRecognize(const QString& userImgPath,
                                int &outLabel, int& contentCode,
                                QString& contentName, int expectLabel) = 0;
};

#define HandWrittenInterface_iid "com.ubtrobot.HandWrittenInterface/1.0"
Q_DECLARE_INTERFACE(HandWrittenInterface, HandWrittenInterface_iid)

#endif // HANDWRITTENINTERFACE_H
