#ifndef FILEDIRHANDLER_H
#define FILEDIRHANDLER_H

#include "utilsGlobal.h"

#include <QCoreApplication>
#include <QString>

class UTILS_EXPORT FileDirHandler
{
public:
    static const QString DIR_SPLIT_CHAR;

public:
    FileDirHandler() = delete;

    static QString absolutePath(const QString &path, const QString &relative = QCoreApplication::applicationDirPath());
    static bool copyDir(const QString& srcPath, const QString& destPath);
};

#endif // FILEDIRHANDLER_H
