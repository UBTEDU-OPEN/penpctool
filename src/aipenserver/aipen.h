#ifndef AIPEN_H
#define AIPEN_H

#include <QObject>
#include <vector>
#include <memory>

#include "PenDataHandleFactory.h"
#include "BaseEntity.h"
//#include "basic_api.h"
#include "bookinfo.h"
#include "ocrcharhandler.h"

class  AiPen : public QObject
{
    Q_OBJECT
public:
    explicit AiPen(QString mac, QObject *parent = nullptr);
    PenDataHandle* dataHandle() { return dataHandle_; }
    void setHandle(PenDataHandle* handle) { dataHandle_ = handle; }
    void onNewPoints(int strokeId, std::vector<std::shared_ptr<Point>> points);
    void onNewGrid(int bookId_, int pageId_, int xIndex_, int yIndex_, int zIndex_);
    void onResult();
    void onClear();

    void onNotifyStopClassWork();
    void onNotifyStartClassWork();
    void onNotifyFinishClass();
    void wordEvaluate(const WordDetail& detail, std::vector<Stroke>& strokes, int workClassId);

    void handleNewDictationWord(int groupId, const QVector<int>& contentCodes);
    void onPostOCRChar(int groupId, int order, int contentCode, OCRChar ocrChar);
    void handleDictationStop();

private:
    void wordEvaluate();
    void generateFileName(QString mac, QString wordCode, QString& dateStr, QString& fileName);
    bool saveImage(const QString& filePath, std::vector<Stroke>& strokes);
    int getWordDetail(WordDetail& detail);
    QByteArray buildUploadData(int groupId, int order, int contentCode, const OCRChar& info);

private:
    QString macAddr_;
    PenDataHandle* dataHandle_;
    std::vector<Stroke>* strokes_;
    int currentWordCode_;
    bool waitingWordCode_;
    int bookId_;
    int pageId_;
    int xIndex_;
    int yIndex_;
    int zIndex_;

    int currentWorkClassWordIndex_; //
    bool newGridReady_; //新的grid一定要先来才会处理后面的点集

    OCRCharHandler ocrHandler_;
};

#endif // AIPEN_H
