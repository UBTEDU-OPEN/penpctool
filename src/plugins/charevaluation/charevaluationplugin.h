#ifndef CHAREVALUATION_H
#define CHAREVALUATION_H

#include <QObject>

#include "charevaluationinterface.h"

class CharEvaluationPlugin : public QObject, public CharEvaluationInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.ubtrobot.CharEvaluationPlugin" FILE "charevaluation.json")
    Q_INTERFACES(CharEvaluationInterface)

public:
    explicit CharEvaluationPlugin(QObject *parent = nullptr);

    bool evaluate(EvaluationRequest& request, EvaluateReportBody& body) override;

};

#endif // CHAREVALUATION_H
