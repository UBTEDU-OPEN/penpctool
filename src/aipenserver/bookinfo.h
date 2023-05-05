#ifndef BOOKINFO_H
#define BOOKINFO_H
#include <QObject>
#include <QString>
#include <QMap>
#include "evaluationserializer.h"

class BookInfo
{
public:
    BookInfo();
    QMap<int,QMap<int,WordDetail>> pageMap;

    //返回wordCode
    int getWordDetail(int pageId,int zIndex, WordDetail& wordDetail);
};

#endif // BOOKINFO_H
