#ifndef MODULEINFO_H
#define MODULEINFO_H

#include <QString>

class ModuleInfo
{
public:
    ModuleInfo();

    enum ModuleType {
        kCharEvaluation = 1,
        kHandWritten,
        kAipenServer
    };

    static QString getVersion(ModuleType moduleType);
    static int compareVersion(QString oldVer, QString newVer);

};

#endif // MODULEINFO_H
