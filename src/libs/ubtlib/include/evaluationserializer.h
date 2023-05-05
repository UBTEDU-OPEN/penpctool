#ifndef EVALUATION_SERIALIZER_H
#define EVALUATION_SERIALIZER_H

#include <vector>
#include <string>

#include "evaluationwrapper_global.h"

struct UPoint
{
    double x;
    double y;
    long long time;
    int type;
    double width;
};

struct UStroke
{
    int strokeId;
    std::vector<std::shared_ptr<UPoint>> pointList;
};

struct  WordDetail{
    int contentCode = 0;
    int contentId = 0;
    int structId = 0;
    int strokeNum = 0;
    int wordOrder = -1;
    int lessonId = -1;
    std::string contentName;
    std::string characterXmlUrl;//xml key path
    std::string characterXmlUpdateTime; //xml 更新时间
} ;

struct  EvaluationRequest {
    bool isWorkClass;
    int classId;
    std::string mac;
    std::string imgKey;
    std::string imgPath;
    std::string jsonKey;
    std::string jsonPath;
    WordDetail wordDetail;
    std::vector<UStroke> strokeList;
};

struct EvaluationResponse {
    std::string mac;
    std::string imgKey;
    std::string imgPath;
    std::string jsonKey;
    std::string jsonPath;
    std::string data;
};

struct EvaluateError1
{
    std::string errorCode;
    int errorType;
    double errorScore;
};

struct EvaluateReportBody
{
    int contentCode = 0;//字的code
    int contentId = 0;
    int wordOrder = -1;
    std::string contentName;
    std::vector<EvaluateError1>  errList;//评价信息
    int isCorrect = 0;//评价字正确与否
    int lessonId = 0;
    std::string practiseBriefInfo;
    int practiseType = 1;//1学情 2作业 (点集开始作业之前到收作业之间是作业,否则是学情)
    std::string practiseXmlUrl;//评价字url
    int score = 0;//评价字分数
    std::string thumbnailUrl;//评价字图片url
    int workClassId = 0;
    bool strokeNumCorrect = false;
    bool workClass = false;
    std::string mac;
};

EVALUATION_WRAPPER_DLL_EXPORT std::string serializeEvaluationRequest(const WordDetail &detial, std::vector<UStroke> &strokes,
                                          std::string mac, int classId, bool isWorkClass,
                                          std::string imgKey, std::string imgPath, std::string jsonKey,
                                          std::string jsonPath);
EVALUATION_WRAPPER_DLL_EXPORT int unserializeEvaluationRequest(const std::string& data, EvaluationRequest &request);
EVALUATION_WRAPPER_DLL_EXPORT std::string serializeEvaluationResponse(std::string mac, std::string imgKey, std::string imgPath, 
                                        std::string jsonKey, std::string jsonPath, std::string evaluateResult);
EVALUATION_WRAPPER_DLL_EXPORT int unserializeEvaluationResponse(const std::string& data, EvaluationResponse &response);

#endif // EVALUATION_SERIALIZER_H
