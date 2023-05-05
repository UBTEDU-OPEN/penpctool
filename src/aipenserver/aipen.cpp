#include "aipen.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QArrayData>
#include <QFile>
#include <QDateTime>
#include <ctime>
#include <QPixmap>
#include <QPainter>
#include <QtMath>
#include <json/json.h>
#include <QRandomGenerator>

#include "logHelper.h"
#include "httpconfig.h"
#include "ubtserver.h"
#include "bookinfo.h"
#include "aipenmanager.h"
#include "alyservice.h"
#include "evaluationserializer.h"
#include "config.h"

const int kInvalidId = -1;

AiPen::AiPen(QString mac, QObject *parent) : QObject(parent)
  , macAddr_(mac)
  , currentWordCode_(kInvalidId)
  , waitingWordCode_(false)
  , bookId_(kInvalidId)
  , pageId_(kInvalidId)
  , xIndex_(kInvalidId)
  , yIndex_(kInvalidId)
  , zIndex_(kInvalidId)
  , currentWorkClassWordIndex_(1)
  , newGridReady_(false)
  , ocrHandler_(mac)
{
    strokes_ = new std::vector<Stroke>;

    connect(&ocrHandler_,&OCRCharHandler::postOCRChar,this,&AiPen::onPostOCRChar);

}

void AiPen::onNewPoints(int strokeId, std::vector<std::shared_ptr<Point>> points)
{
    if(AiPenManager::showVerboseLog_) {
        LOG(INFO) << "AiPen::onNewPoints " << macAddr_.toStdString() << " stroke id=" << strokeId
                  << ",point size=" << points.size() << ",current stroke size=" << strokes_->size();

        for(auto point : points) {
            LOG(INFO) << macAddr_.toStdString() << " strokeId=" << strokeId
                      << ",pos=" << point->getX() << "," << point->getY() << ",type=" << point->getType() << point->getTime();
        }
    }

    if(strokeId < 0) {
        LOG(INFO) << "AiPen::onNewPoints invalid stroke id=" << strokeId;
        return;
    }

    if(!newGridReady_) {
        LOG(INFO) << macAddr_.toStdString() << " can not process new points before a new grid." << strokeId;
        return;
    }

    unsigned unStrokeId = static_cast<unsigned>(strokeId);
    unsigned strokeSize = strokes_->size();
    if(unStrokeId > strokeSize) { //无效数据
        LOG(WARNING) << "invalid strokeId:" << unStrokeId;
        return;
    }

    if(unStrokeId == strokeSize) { //下一笔
        Stroke stroke;
        stroke.setStrokeId(strokeId);
        strokes_->push_back(stroke);
    }

    for(auto point : points) {
        (*strokes_)[unStrokeId].addPoint(point);
    }
}

void AiPen::onNewGrid(int bookId, int pageId, int xIndex, int yIndex, int zIndex)
{
    LOG(INFO) << "onNewGrid mac=" << macAddr_.toStdString() << "," << bookId << "," << pageId << "," << xIndex << "," << yIndex << "," << zIndex;
    //先对上一个字进行评价
    if(!strokes_->empty()) {
        LOG(INFO) << "onNewGrid mac=" << macAddr_.toStdString() << ",old word need to evaluate first.";

        wordEvaluate();
    }
    strokes_->clear();
    newGridReady_ = true;
    //记录新字
    bookId_ = bookId;
    pageId_ = pageId + 1;
    xIndex_ = xIndex;
    yIndex_ = yIndex;
    zIndex_ = zIndex + 1; //TODO: 讲话
}

void AiPen::onResult()
{
    LOG(INFO) << "onResult ";
    wordEvaluate();
}

void AiPen::onClear()
{
    LOG(INFO) << "onClear";
    strokes_->clear();
}

int AiPen::getWordDetail(WordDetail& detail)
{
    if(AiPenManager::getInstance()->dictationStarted()) {
        detail.contentCode = AiPenManager::getInstance()->getDictationGroupId();
        return detail.contentCode;
    } else if(AiPenManager::getInstance()->isStartedClasswork()) {
        int wordCode =  AiPenManager::getInstance()->getWordDetail(currentWorkClassWordIndex_,detail);
        ++currentWorkClassWordIndex_;
        return wordCode;
    } else {
        return AiPenManager::getInstance()->getWordDetail(bookId_,pageId_,zIndex_,detail);
    }
}

void AiPen::wordEvaluate()
{
    newGridReady_ = false;
    LOG(INFO) << "AiPen::wordEvaluate222 mac=" << macAddr_.toStdString() << ", stroke num=" << strokes_->size();
    if(strokes_->empty()) {
        return;
    }
    WordDetail detail;
    int wordCode = 0;
    if(Config::testRestoreMode()) {
        wordCode = 4812;
        detail.contentCode = 4812;
    } else {
        wordCode = getWordDetail(detail);
    }

    if(0 == wordCode) {
        LOG(INFO) << macAddr_.toStdString() << " can not get word code";
        if(AiPenManager::getInstance()->isStartedClasswork() ||
                AiPenManager::getInstance()->invalidBookId(bookId_)) {
            LOG(INFO) << macAddr_.toStdString() << " invalid bookId:" << bookId_
                      << ", or class work already completed, current index=" << currentWorkClassWordIndex_;
            strokes_->clear();
        } else {
            auto tempPtr = strokes_;
            strokes_ = new std::vector<Stroke>();
            emit AiPenManager::getInstance()->pendingWord(macAddr_,bookId_,pageId_,xIndex_,yIndex_,zIndex_,reinterpret_cast<qulonglong>(tempPtr));
        }
    } else {
        if(!AiPenManager::getInstance()->dictationStarted() &&
                !AiPenManager::getInstance()->isStartedClasswork() &&
                AiPenManager::getInstance()->getWorkClassId(detail.lessonId) < 0) {
            auto tempPtr = strokes_;
            strokes_ = new std::vector<Stroke>();
            emit AiPenManager::getInstance()->pendingWord(macAddr_,bookId_,pageId_,xIndex_,yIndex_,zIndex_,reinterpret_cast<qulonglong>(tempPtr));
        } else {
            int classId = AiPenManager::getInstance()->getClassId();
            if(!AiPenManager::getInstance()->isStartedClasswork()) {
                classId = AiPenManager::getInstance()->getWorkClassId(detail.lessonId);
            }

            wordEvaluate(detail,*strokes_,classId);
            LOG(INFO) << "AiPen::wordEvaluate clear stroke";
            strokes_->clear();
        }
    }
}

void AiPen::generateFileName(QString mac, QString wordCode, QString& dateStr, QString& fileName)
{
    QDateTime cur = QDateTime::currentDateTime();
    dateStr = cur.toString("yyyyMMdd");
    QString dateTimeStr = cur.toString("yyyyMMddhhmmss");
    quint32 randomNumber = 0;
    static quint32 index = 0;
    if(Config::testRestoreMode()) {
        randomNumber = index++;
        randomNumber %= 999999;
    } else {
        randomNumber = QRandomGenerator::global()->generate()%999999;
    }

    QString randomNumberStr = QString("%1").arg(randomNumber,6,10,QLatin1Char('0'));
    QString tempMac = mac;
    fileName = dateTimeStr + "_" + wordCode + "_" + tempMac.remove(':') + "_" + randomNumberStr;
}

void AiPen::wordEvaluate(const WordDetail& detail, std::vector<Stroke>& strokes, int workClassId)
{
    LOG(INFO) << "AiPen::wordEvaluate111 mac=" << macAddr_.toStdString();

    QString timeStr;
    QString fileName;

    generateFileName(macAddr_,QString::number(detail.contentCode),timeStr,fileName);
    QString alyKeyBase = QString::fromStdString(HttpConfig::instance()->getAlyKeyBasePath());
    QString imgKey = alyKeyBase + "practiceThumbnail/" + timeStr +"/"+fileName;
    QString imgPath = Config::getTempPath()+fileName+".png";
    bool imgOk = saveImage(imgPath,strokes);
    LOG(INFO) << "AiPen::wordEvaluate " << imgPath.toStdString() << " save ret=" << imgOk;
    QString jsonKey = alyKeyBase + "practiceJsonFile/" + timeStr + "/" + fileName;
    QString jsonPath = Config::getTempPath()+fileName+".json";
    if(Config::testRestoreMode()) {
        return;
    }

    if(imgOk) {
        if(AiPenManager::getInstance()->dictationStarted()) {
            int preLabel = 0;
            int realContentCode = 0;
            int score = 0;
            QString contentName;
            if(nullptr != AiPenManager::getInstance()->hwInterface_) {
                score = AiPenManager::getInstance()->hwInterface_->singleRecognize(imgPath,preLabel,realContentCode,contentName,-1);
                LOG(INFO) << "AiPen::wordEvaluate label=" << preLabel << ",score=" << score
                          << ",contentCode=" << realContentCode;
            } else {
                LOG(WARNING) << "hwInterface_ is null";
            }
            if(0 == realContentCode) {
                LOG(WARNING) << macAddr_.toStdString() << " wordEvaluate wrong label";
            } else {
//                LOG(INFO) << "wordEvaluate KEY before:" << imgKey.toStdString();
                imgKey.replace("_" + QString::number(detail.contentCode) + "_",
                               "_" + QString::number(realContentCode) + "_");
//                LOG(INFO) << "wordEvaluate KEY after:" << imgKey.toStdString();
                ocrHandler_.addOCRChar(pageId_,xIndex_,yIndex_,zIndex_,realContentCode,
                                       contentName,score,imgKey,imgPath);
            }
        } else {
            std::vector<UStroke> tempStrokes;
            for(auto& stroke : strokes) {
                UStroke us;
                us.strokeId = stroke.getStrokeId();
                const auto pointList = stroke.getPointList();
                for(const auto& point : pointList) {
                    auto up = std::make_shared<UPoint>();
                    up->type = point->getType();
                    up->x = point->getX();
                    up->y = point->getY();
                    up->time = point->getTime();
                    up->width = point->getWidth();
                    us.pointList.push_back(up);
                }
                tempStrokes.push_back(us);
            }

            QByteArray bytes = QByteArray::fromStdString(serializeEvaluationRequest(detail, tempStrokes, macAddr_.toStdString(),
                                                          workClassId,true,
                                                          imgKey.toStdString(),imgPath.toStdString(),
                                                          jsonKey.toStdString(),jsonPath.toStdString()));
            LOG(INFO) << "AiPen::wordEvaluate serializeEvaluationRequest:" /*<< bytes.data()*/;
            emit AiPenManager::getInstance()->requestEvaluate(bytes);
        }
    }
}

void AiPen::handleNewDictationWord(int groupId, const QVector<int>& contentCodes)
{
    ocrHandler_.handleNewDictationWord(groupId,contentCodes);
}

void AiPen::onPostOCRChar(int groupId, int order, int contentCode, OCRChar ocrChar)
{
    QByteArray bytes = buildUploadData(groupId, order, contentCode, ocrChar);
    emit AiPenManager::getInstance()->postOCRChar((int)ResultType::kOCRResult,
                                                  macAddr_,ocrChar.imgKey,ocrChar.imgPath,"",""
                                                  ,bytes);
}

void AiPen::handleDictationStop()
{
    LOG(INFO) << "payne: onNotifyFinishClass";
    onNotifyFinishClass();
    LOG(INFO) << "payne: finishAllWords";
    ocrHandler_.finishAllWords();
}

bool AiPen::saveImage(const QString& filePath, std::vector<Stroke>& strokes)
{
    QPixmap wordPic(400,400);
    wordPic.fill(Qt::transparent);
    QPainter painter(&wordPic);
    painter.setRenderHints(QPainter::Antialiasing| QPainter::SmoothPixmapTransform, true );
    painter.setPen(QPen(QColor(Qt::black), 1, Qt::SolidLine, Qt::RoundCap/*RoundCap*/, Qt::RoundJoin));
    QLine line;
    for(auto& stroke : strokes) {
        auto& pointList = stroke.getPointList();
        if (pointList.empty()) {
            continue;
        }
        QPoint p1 = QPoint(pointList[0]->getX(),pointList[0]->getY());
        double width1 = pointList[0]->getWidth();
        for(unsigned int i = 1; i < pointList.size(); ++i) {
            QPoint p2 = QPoint(pointList[i]->getX(),pointList[i]->getY());
            double width2 = pointList[i]->getWidth();
            double sWidth = (400.0/45.0)*qPow(width1*0.95+width2*0.05,0.15);
            //LOG(INFO) << "pen width" << sWidth;
            QPen pen = painter.pen();
            pen.setWidth(qRound(sWidth));
            line.setP1(p1);
            line.setP2(p2);
            painter.setPen(pen);
            painter.drawLine(line);
//            painter.drawPoint(p2);
            p1 = p2;
            width1 = width2;
        }
    }
    bool saveRet = wordPic.save(filePath,"PNG");
    LOG(INFO) << "AiPen::saveImage pic=" << filePath.toStdString() << ",save result=" << saveRet;
    return saveRet;
}

QByteArray AiPen::buildUploadData(int groupId, int order, int contentCode, const OCRChar& info)
{
    EvaluateReportBody repoerBody;
    repoerBody.mac = macAddr_.toStdString();
    repoerBody.workClassId = AiPenManager::getInstance()->getClassId();
    repoerBody.contentCode = contentCode;
    repoerBody.wordOrder = order;
    repoerBody.contentName = info.ocrContentName.toStdString();
    repoerBody.isCorrect = info.correct ? 1 : 0;
//    repoerBody.lessonId =
    repoerBody.thumbnailUrl = info.imgKey.toStdString();

    QJsonDocument jdoc;
    QJsonObject baseObj;
    QJsonObject obj;
    QJsonDocument jdoc1;
    QJsonObject obj1;
    obj1["id"] = 0;
    obj1["wordCode"] = repoerBody.contentCode;
    obj1["createTime"] = 500;
    obj1["isCorrect"] = info.correct;
    obj1["workId"] = repoerBody.workClassId;
    jdoc1.setObject(obj1);
    obj["practiseBriefInfo"] = QString(jdoc1.toJson());
    obj["contentCode"] = repoerBody.contentCode;
    obj["ocrContentCode"] = info.ocrContentCode;
    obj["contentName"] = QString::fromStdString(repoerBody.contentName);
    obj["groupId"] = groupId;
    obj["isCorrect"] = repoerBody.isCorrect;
//    obj["score"] = repoerBody.score;
    obj["thumbnailUrl"] = QString::fromStdString(repoerBody.thumbnailUrl);
    obj["wordOrder"] = repoerBody.wordOrder;
    obj["workClassId"] = repoerBody.workClassId;
    baseObj["practiseResultDTO"] = obj;
    baseObj["mac"] = QString::fromStdString(repoerBody.mac);
    jdoc.setObject(baseObj);
    QByteArray bytes = jdoc.toJson(QJsonDocument::Indented);
    LOG(INFO) << "AiPen::buildUploadData data=" << bytes.data();
    return bytes;
}

void AiPen::onNotifyStopClassWork()
{
    LOG(INFO) << "Stop class work"<< macAddr_.toStdString();
    if(!strokes_->empty()) {
        LOG(INFO) << "onNotifyStopClassWork need wordEvaluate"<< macAddr_.toStdString();
        wordEvaluate();
    }
    currentWorkClassWordIndex_ = 1;
}

void AiPen::onNotifyStartClassWork()
{
    LOG(INFO) << "Start class work";
    if(!strokes_->empty()) {
        LOG(INFO) << "onNotifyStartClassWork need wordEvaluate";
        wordEvaluate();
    }
    currentWorkClassWordIndex_ = 1;
}

void AiPen::onNotifyFinishClass()
{
    LOG(INFO) << "AiPen::onNotifyFinishClass" << macAddr_.toStdString();
    if(!strokes_->empty()) {
        LOG(INFO) << "onNotifyFinishClass need wordEvaluate"<< macAddr_.toStdString();
        wordEvaluate();
    }
}
