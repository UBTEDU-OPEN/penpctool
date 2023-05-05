#ifndef CLASSWORK_H
#define CLASSWORK_H
#include <QObject>
#include <QList>

#include "utilsGlobal.h"


typedef struct Practice {
    int times = 0;//次數
    int contentCode = 0;//
    int structureId = 0;//
    int characterStrokesNum = 0;//
    int contentId = 0;//
    int wordOrder = -1; //-1是无效序号
    QString contentName;//
    QString characterXmlUrl;//xml key path
    QString characterXmlUpdateTime; //xml 更新时间
}Practice;

class  UTILS_EXPORT Classwork : public QObject
{
    Q_OBJECT
public:
    explicit Classwork(QObject *parent = nullptr);
    ~Classwork();
public:
    int workClassId;
    int lessonId;
    int practiseType;
    int startTime;
    int endTime;
    QString practiseBriefInfo;
    QList<Practice> practices;
};

#endif // CLASSWORK_H
