#include "fileDirHandler.h"

#include <QDir>
#include <QFileInfo>

const QString FileDirHandler::DIR_SPLIT_CHAR = QDir::separator();

QString FileDirHandler::absolutePath(const QString &path, const QString &relative)
{
    QDir dir(relative);
    return dir.absoluteFilePath(path);
}

bool FileDirHandler::copyDir(const QString &srcPath, const QString &destPath)
{
    QDir srcDir(srcPath);
    QDir destDir(destPath);
    if(destDir.exists()) {
        return false; //目标路径已经存在
    } else {
        destDir.mkdir(destPath);
    }
    if(!srcDir.exists()) {
        return false; //源路径不存在
    }
    QFileInfoList infoList = srcDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    for(QFileInfo info : infoList) {
        if(info.isDir()) {
            copyDir(info.absoluteFilePath(),destDir.absolutePath()+"/"+info.fileName());
        } else if(info.isFile()) {
            QFile::copy(info.absoluteFilePath(),destDir.absolutePath()+"/"+info.fileName());
        }
    }
    return true;
}

