#ifndef AIPENMANAGER_H
#define AIPENMANAGER_H

#include <QObject>
#include "Classwork.h"
#include <QSettings>
#include <QMap>
#include <QSet>
#include <QRecursiveMutex>
#include <QVector>

#include "TQLAPComm.h"
#include "BaseEntity.h"
#include "PenDataHandleFactory.h"
#include "bookinfo.h"
#include "aipen.h"
#include "pendingwordhandlethread.h"
#include "uploaddatathread.h"
#include "readevaluateresultthread.h"
#include "evaluaterequestworker.h"
#include "uploadorigindataworker.h"
#include "handwritteninterface.h"
#include "commondefines.h"

struct WordEvaluate;

enum PredefinedWorkClassId {
    kWCIInit = -2, //初始值
    kWCICreating = -1  //创建中
    //其他正值为实际workClassId
};

enum WorkType {
    kFreePracticeType = 0,
    kClassWorkType,
    kDictationType
};

class  AiPenManager : public QObject
{
    Q_OBJECT
private:
    explicit AiPenManager(QObject *parent = nullptr);
    static AiPenManager* instance_;
    //section
    static const QString aipenmanager;
    //key
    static const QString save_token;
    static const QString start_attended;
    static const QString start_classwork;

    static const QString classwork_id;
    static const QString end_time;
    static const QString lession_id;
    static const QString practise_type;
    static const QString practise_brief_info;

    //array
    static const QString practices;
    static const QString times;
    static const QString content_code;
    static const QString content_id;
    static const QString content_name;
    static const QString structure_id;
    static const QString character_strokes_num;
    static const QString character_xml_url;

public:
    static AiPenManager* getInstance();
    ~AiPenManager();

    void setReceivedHeartbeatPacket();
    bool hasReceivedHeartbeatPacket(){ return receivedHeartbeatPacket; }

    bool isStartedAttend() { return startAttended_; }
    bool isStartedClasswork(){ return (startAttended_ && startClasswork_); }

    static void onPenDown(const std::string& mac);
    static void onPenClear(const std::string& mac);
    static void onPenResult(const std::string& mac);
    static void onPenNewPoint(const std::string& mac,
                              int strokeId,
                              std::vector<std::shared_ptr<Point>>& newPoints);
    static void onPenNewGrid(const std::string& mac, int bookId, int pageId, int xIndex, int yIndex, int zIndex);
    static void onPenStrokeInterval(const std::string& mac, int strokeId, long long timeInterval);
    static void onPenWordInterval(const std::string& mac, long long timeInterval);
    static bool showVerboseLog_;

    static QMap<QString,AiPen*> penHandleMap;

    /*获取字帖信息*/
    void requestBookWordDetail(int bookId);
    int getWordDetail(int bookId, int pageId, int zIndex, WordDetail& detail);
    int getWordDetail(int wordIndex, WordDetail& detail);
    bool invalidBookId(int bookId);
    int getClassId() { return workClassId_; }
    void handleDot(QString mac, int x, int fx, int y, int fy, int bookId, int pageId, int type, int force, long long timeStamp);

    void stopClassOnExit();
    int getWorkClassId(int lessonId);


    void finishAllClassWork();

    bool dictationStarted();
    int getDictationGroupId();

    HandWrittenInterface* hwInterface_ = nullptr;

signals:
    void newPoint(double x, double y);
    void httpRequest(QString,QByteArray,int,int,qulonglong);

    void pendingWord(QString mac, int bookId, int pageId, int xIndex, int yIndex, int zIndex, qulonglong strokes);
    void postResult(int result, qulonglong tempData);
    void requestEvaluate(QByteArray wordData);
    void uploadOriginData(int workClassId, QString classTimeStamp);
    void updateWordXml(int wordCode, QString updateTime, QString urlKey, bool bookDetail);
    void postOCRChar(int type, QString mac, QString imgKey, QString imgPath,
                     QString jsonKey, QString jsonPath, QByteArray evaluateResult);

public slots:
    void onNotifyAttendClass(int classId);
    void onNotifyFinishClass();
    void onNotifyStartClasswork(Classwork* classwork);
    void onNotifyStopClasswork();
    void onNotifyStartDictation(int workClassId);
    void onNotifyStopDictation();
    void onDictationWordStart(int groupId, QString contentCode);
    void onGetBookDetail(int result,const QJsonObject& js1, qulonglong tempData);
    void onGetWorkClassId(int result,const QJsonObject& js1, qulonglong tempData);

    void finishLastClasswork(); //结束未完成的作业，包括听写作业

//signals:
//    void sendTextMessage(QWebSocket* client, QString msg);


private:
    bool startAttended_;//开始上课了
    bool startClasswork_;//开始作业了
    int curClassId_ = -1;
    bool dictationStarted_ = false;

    //作业数据
    int workClassId_;
    QString classTimeStamp_;
    qlonglong endTime_;
    int lessonId_;
    int practiseType_;
    QString practiseBriefInfo_;
    QList<Practice> practices_;
    QMap<int,BookInfo> bookInfoMap_;
    QSet<int> requestingBooks_;
    QSet<int> invalidBooks_;

    bool receivedHeartbeatPacket = false;//能判断一个AP

    PendingWordHandleThread* wordHandler_;
    UploadDataThread* uploadThread_;

    QThread threadObjsLivingThread_; //避免对象在主线程处理东西
    QRecursiveMutex bookMutex_;
    static QRecursiveMutex penMapMutex_;

    ReadEvaluateResultThread* readResultThread_;

    EvaluateRequestWorker* evaluateRequestWorker_;
    QThread evaluateRequestThread_;
    UploadOriginDataWorker* uploadOriginDataWorker_;
    QThread uploadOriginDataThread_;
    QMap<int,int> lessonIdToWorkClassId_;
    QRecursiveMutex workClassIdMutex_;
    DictationWord* currentDictationWord_ = nullptr;
    QList<DictationWord*> dictationWordList_;
    QRecursiveMutex dictationWordListMutex_;
};

#endif // AIPENMANAGER_H
