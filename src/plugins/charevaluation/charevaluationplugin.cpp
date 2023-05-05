#include "charevaluationplugin.h"

#include "charevaluationwrapper.h"

CharEvaluationPlugin::CharEvaluationPlugin(QObject *parent)
    : QObject(parent)
{
}

bool CharEvaluationPlugin::evaluate(EvaluationRequest& request, EvaluateReportBody& body)
{
    return CharEvaluationWrapper::instance()->wordEvaluate(request,body);
}

