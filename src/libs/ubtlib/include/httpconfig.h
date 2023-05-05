#ifndef ENVCONFIG_H
#define ENVCONFIG_H

#include <string>

#include "portlib_global.h"

enum class Env {
    test = 0,
    prerelease,
    release
};

class PORTLIB_DLL_EXPORT HttpConfig {
public:
    static HttpConfig* instance();
    void setEnv(Env type);
    std::string getUpdataFile();
    std::string getEvaluateResult();
    std::string getWorkCommit();
    std::string getBookInfoDetail(int pageId);
    std::string getAlyKeyBasePath();
    std::string getSideXmlUrl();
    std::string getDownloadUrl(const std::string& urls, int validTime);
    std::string getUpgradeUrl(const std::string& moduleName, const std::string& productName,const std::string& version);
    std::string getCreateWorkClassUrl(int lessonId, int classId);
    std::string getFinishAllWorkClassUrl();
    std::string getDictationResult();

private:
    HttpConfig();
    Env envType_ = Env::test;
    std::string baseUrl_;
};

#endif // ENVCONFIG_H
