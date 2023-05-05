#ifndef HAND_WRITTEN_WRAPPER_H
#define HAND_WRITTEN_WRAPPER_H

#include <mutex>
#include <string>

#include "handwrittenwrapper_global.h"

class HAND_WRITTEN_WRAPPER_DLL_EXPORT HandWrittenWrapper {
public:
    static HandWrittenWrapper* instance();
    int singleRecognize(const std::string& userImgPath,int &outLabel, int& contentCode, std::string& contentName, int expectLabel);

private:
    HandWrittenWrapper();
    static std::once_flag instFlag_;
    static HandWrittenWrapper* instance_;
};


#endif //HAND_WRITTEN_WRAPPER_H