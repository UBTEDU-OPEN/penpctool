#ifndef CHAR_EVALUATION_WRAPPER_H
#define CHAR_EVALUATION_WRAPPER_H

#include <mutex>
#include "evaluationserializer.h"

#include "evaluationwrapper_global.h"

class EVALUATION_WRAPPER_DLL_EXPORT CharEvaluationWrapper {
public:
    static CharEvaluationWrapper* instance();
    bool wordEvaluate(EvaluationRequest& request, EvaluateReportBody& body);

private:
    CharEvaluationWrapper() = default;
    static std::once_flag instFlag_;
    static CharEvaluationWrapper* instance_;
};


#endif //CHAR_EVALUATION_WRAPPER_H