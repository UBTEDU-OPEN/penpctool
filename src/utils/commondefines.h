#ifndef COMMONDEFINES_H
#define COMMONDEFINES_H

#include <QVector>

struct DictationWord {
    int groupId;
    QVector<int> contentCodes;
};

#endif // COMMONDEFINES_H
