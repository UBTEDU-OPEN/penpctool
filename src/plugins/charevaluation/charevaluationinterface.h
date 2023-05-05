#ifndef CHAREVALUATIONINTERFACE_H
#define CHAREVALUATIONINTERFACE_H

#include <vector>
#include <QString>
#include <QObject>

#include "evaluationserializer.h"


class CharEvaluationInterface
{
public:
    virtual ~CharEvaluationInterface() = default;

    virtual bool evaluate(EvaluationRequest& request, EvaluateReportBody& body) = 0;
};

#define CharEvaluationInterface_iid "com.ubtrobot.CharEvaluationInterface/1.0"
Q_DECLARE_INTERFACE(CharEvaluationInterface, CharEvaluationInterface_iid)

#endif // CHAREVALUATIONINTERFACE_H
