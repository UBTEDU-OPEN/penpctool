#include "aipenmanager.h"
#include "QCoreApplication"
#include "aipen.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QPluginLoader>

#include "tqlsdkadapter.h"
//#include "WordEvaluate.h"
#include "logHelper.h"
//#include "word_evaluate.h"
#include "httpconfig.h"
#include "ubtserver.h"
#include "config.h"
#include "projectconst.h"

// section
const QString AiPenManager::aipenmanager="aipenmanager";
// key
const QString AiPenManager::save_token="token";
const QString AiPenManager::start_attended="start_attended";
const QString AiPenManager::start_classwork="start_classwork";
const QString AiPenManager::classwork_id="workClassId";
const QString AiPenManager::end_time="endTime";
const QString AiPenManager::lession_id="lessonId";
const QString AiPenManager::practise_type="practiseType";
const QString AiPenManager::practise_brief_info="practiseBriefInfo";

const QString AiPenManager::practices="words";
const QString AiPenManager::times="times";
const QString AiPenManager::content_code="contentCode";
const QString AiPenManager::content_id="contentId";
const QString AiPenManager::content_name="contentName";
const QString AiPenManager::structure_id="structureId";
const QString AiPenManager::character_strokes_num="characterStrokesNum";
const QString AiPenManager::character_xml_url="characterXmlUrl";

QMap<QString,AiPen*> AiPenManager::penHandleMap;
QRecursiveMutex AiPenManager::penMapMutex_;

bool AiPenManager::showVerboseLog_ = false;

AiPenManager::AiPenManager(QObject *parent) : QObject(parent)
{
    if(Config::testRestoreMode()) {
        startAttended_ = true;
        startClasswork_ = true;
        workClassId_ = 123;
    } else {
        Config::getClassInfo(startAttended_, curClassId_);
        Config::getClassWorkInfo(startClasswork_, workClassId_);//启动时从配置里面读取状态，这样
    }
    qRegisterMetaType<OCRChar>("OCRChar");
    showVerboseLog_ = (1 == Config::getLogVerbose());
    wordHandler_ = new PendingWordHandleThread;
    wordHandler_->moveToThread(&threadObjsLivingThread_);
    connect(this,&AiPenManager::pendingWord,wordHandler_,&PendingWordHandleThread::handleWord);
    wordHandler_->start();

    uploadThread_ = new UploadDataThread;
    uploadThread_->moveToThread(&threadObjsLivingThread_);
    connect(this,&AiPenManager::postResult,uploadThread_,&UploadDataThread::onPostResult);
    connect(uploadThread_,&UploadDataThread::httpRequest,this,&AiPenManager::httpRequest);
    uploadThread_->start();

    readResultThread_ = new ReadEvaluateResultThread;
    connect(readResultThread_,&ReadEvaluateResultThread::wordResultReady,uploadThread_,&UploadDataThread::onNewWord);
    readResultThread_->start();

    connect(this,&AiPenManager::postOCRChar,uploadThread_,&UploadDataThread::onNewWord);

    threadObjsLivingThread_.start();

    evaluateRequestWorker_ = new EvaluateRequestWorker;
    evaluateRequestWorker_->moveToThread(&evaluateRequestThread_);
    connect(this,&AiPenManager::requestEvaluate,evaluateRequestWorker_,&EvaluateRequestWorker::wordEvaluationRequest);
    evaluateRequestThread_.start();

    uploadOriginDataWorker_ = new UploadOriginDataWorker;
    uploadOriginDataWorker_->moveToThread(&uploadOriginDataThread_);
    connect(this,&AiPenManager::uploadOriginData,uploadOriginDataWorker_,&UploadOriginDataWorker::uploadOriginData);
    uploadOriginDataThread_.start();

    QPluginLoader loader("handwrittenplugin.dll");
    hwInterface_ = qobject_cast<HandWrittenInterface*>(loader.instance());
    LOG(INFO) << "handwrittenplugin:" << loader.errorString().toStdString();
    QJsonObject json = loader.metaData().value("MetaData").toObject();
    LOG(INFO) << "";
    LOG(INFO) << "********** MetaData **********";
    LOG(INFO) << json.value("type").toInt();
    LOG(INFO) << json.value("update_time").toString().toStdString();
    LOG(INFO) << json.value("name").toString().toStdString();
    LOG(INFO) << json.value("version").toString().toStdString();
    LOG(INFO) << "hwInterface_=" << hwInterface_;
}

AiPenManager::~AiPenManager()
{
    wordHandler_->stopHandle();
    wordHandler_->quit();
    wordHandler_->wait();
    wordHandler_->deleteLater();
    uploadThread_->stopUpload();
    uploadThread_->quit();
    uploadThread_->wait();
    uploadThread_->deleteLater();
    threadObjsLivingThread_.quit();
    threadObjsLivingThread_.wait();
    readResultThread_->stopRunning();
    readResultThread_->wait();

    evaluateRequestWorker_->stopRequest();
    evaluateRequestThread_.quit();
    evaluateRequestThread_.wait();

    uploadOriginDataThread_.quit();
    uploadOriginDataThread_.wait();

    LOG(INFO) <<"~AiPenManager()";
}

AiPenManager* AiPenManager::getInstance()
{
    static AiPenManager* instance_ = new AiPenManager;
    return instance_;
}

void AiPenManager::setReceivedHeartbeatPacket()
{
    LOG(INFO)<<"***update receivedHeartbeatPacket*** startAttended_ = " <<startAttended_ << " receivedHeartbeatPacket" << receivedHeartbeatPacket ;
    if(startAttended_ && !receivedHeartbeatPacket)
    {
        LOG(INFO)<<"***update receivedHeartbeatPacket***";
        Config::SetHeartbeat(true);
        receivedHeartbeatPacket = true;
    }
}

void AiPenManager::onNotifyAttendClass(int classId)
{
    startAttended_ = true;
    curClassId_ = classId;
}

void AiPenManager::finishAllClassWork()
{
    {
        QMutexLocker locker(&workClassIdMutex_);
        lessonIdToWorkClassId_.clear();
    }
    emit httpRequest(QString::fromStdString(HttpConfig::instance()->getFinishAllWorkClassUrl()),
                     QByteArray(),1,MyRequestType::kFinishAllWorkClass,0);
}

bool AiPenManager::dictationStarted()
{
    return (startAttended_ && dictationStarted_);
}

int AiPenManager::getDictationGroupId()
{
    if(dictationStarted_ && nullptr != currentDictationWord_) {
        return currentDictationWord_->groupId;
    }

    return -1;
}

void AiPenManager::onNotifyFinishClass()
{
//    QList<AiPen*> penList;
//    {
//        QMutexLocker locker(&penMapMutex_);
//        penList = penHandleMap.values();
//    }
//    for(AiPen* pen : penList) {
//        pen->onNotifyFinishClass();
//    }
    finishLastClasswork();
    startAttended_ = false;
    startClasswork_ = false;
    dictationStarted_ = false;
    receivedHeartbeatPacket = false;
    //下课后重新开始创建
    finishAllClassWork();
}

void AiPenManager::onNotifyStartClasswork(Classwork* classwork)
{
    finishLastClasswork();
    QList<AiPen*> penList;
    {
        QMutexLocker locker(&penMapMutex_);
        penList = penHandleMap.values();
    }
    for(AiPen* pen : penList) {
        pen->onNotifyStartClassWork();
    }

    startClasswork_ = true;

    workClassId_ = classwork->workClassId;
    endTime_ = classwork->endTime;
    lessonId_ = classwork->lessonId;
    practiseType_ = classwork->practiseType;
    practiseBriefInfo_ = classwork->practiseBriefInfo;
    practices_ = classwork->practices;
    for(auto practice : practices_){
        emit updateWordXml(practice.contentCode,practice.characterXmlUpdateTime,practice.characterXmlUrl,false);
    }
    classTimeStamp_ = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");


    classwork->deleteLater();
}

void AiPenManager::onNotifyStopClasswork()
{
    QList<AiPen*> penList;
    {
        QMutexLocker locker(&penMapMutex_);
        penList = penHandleMap.values();
    }
    for(AiPen* pen : penList) {
        pen->onNotifyStopClassWork();
    }

    startClasswork_ = false;

    if(1 == Config::getUploadOrigin()) {
        emit uploadOriginData(workClassId_,classTimeStamp_);
    }
}

void AiPenManager::onNotifyStartDictation(int workClassId)
{
    finishLastClasswork();
    dictationStarted_ = true;
    workClassId_ = workClassId;
}

void AiPenManager::onNotifyStopDictation()
{
    QList<AiPen*> penList;
    {
        QMutexLocker locker(&penMapMutex_);
        penList = penHandleMap.values();
    }
    for(AiPen* pen : penList) {
        pen->handleDictationStop();
    }

    {
        QMutexLocker locker(&dictationWordListMutex_);
        for(auto ptr : dictationWordList_) {
            delete ptr;
        }
        dictationWordList_.clear();
    }
    currentDictationWord_ = nullptr;
    dictationStarted_ = false;
}

void AiPenManager::onDictationWordStart(int groupId, QString contentCode)
{
    QList<AiPen*> penList;
    {
        QMutexLocker locker(&penMapMutex_);
        penList = penHandleMap.values();
    }
    currentDictationWord_ = new DictationWord;
    currentDictationWord_->groupId = groupId;
    QStringList contentCodeList = contentCode.split(",");
    for(QString code : contentCodeList) {
        currentDictationWord_->contentCodes.push_back(code.toInt());
    }

    {
        QMutexLocker locker(&dictationWordListMutex_);
        dictationWordList_.push_back(currentDictationWord_);
    }
    for(AiPen* pen : penList) {
        pen->handleNewDictationWord(currentDictationWord_->groupId,currentDictationWord_->contentCodes);
    }

}

void AiPenManager::onPenDown(const std::string& mac)
{
    qDebug() << "onPenDown" << mac.c_str();
}

void AiPenManager::onPenClear(const std::string& mac)
{
    LOG(INFO) << "AiPenManager::onPenClear pen=" << mac;
    AiPen* pen = nullptr;
    auto macStr = QString::fromStdString(mac);
    {
        QMutexLocker locker(&penMapMutex_);
        if(penHandleMap.contains(macStr)) {
            pen = penHandleMap[macStr];
        } else {
            LOG(INFO) << "AiPenManager::onPenClear no pen=" << mac;
        }
    }
    if(nullptr != pen) {
        pen->onClear();
    }
}

void AiPenManager::onPenResult(const std::string& mac)
{
    LOG(INFO) << "AiPenManager::onPenResult pen=" << mac;
    AiPen* pen = nullptr;
    auto macStr = QString::fromStdString(mac);
    {
        QMutexLocker locker(&penMapMutex_);
        if(penHandleMap.contains(macStr)) {
            pen = penHandleMap[macStr];
        } else {
            LOG(INFO) << "AiPenManager::onPenResult no pen=" << mac;
        }
    }
    if(nullptr != pen) {
        pen->onResult();
    }
}

void AiPenManager::onPenNewPoint(const std::string& mac,
                          int strokeId,
                          std::vector<std::shared_ptr<Point>>& newPoints)
{
    LOG(INFO) << "AiPenManager::onPenNewPoint pen=" << mac;
    AiPen* pen = nullptr;
    auto macStr = QString::fromStdString(mac);
    {
        QMutexLocker locker(&penMapMutex_);
        if(penHandleMap.contains(macStr)) {
            pen = penHandleMap[macStr];
        } else {
            LOG(INFO) << "AiPenManager::onPenNewPoint no pen=" << mac;
        }
    }
    if(nullptr != pen) {
        pen->onNewPoints(strokeId,newPoints);
    }
}

void AiPenManager::onPenNewGrid(const std::string& mac, int bookId, int pageId, int xIndex, int yIndex, int zIndex)
{
    LOG(INFO) << "AiPenManager::onPenNewGrid pen=" << mac;
    AiPen* pen = nullptr;
    auto macStr = QString::fromStdString(mac);
    {
        QMutexLocker locker(&penMapMutex_);
        if(penHandleMap.contains(macStr)) {
            pen = penHandleMap[macStr];
        } else {
            LOG(INFO) << "AiPenManager::onPenNewGrid no pen=" << mac;
        }
    }
    if(nullptr != pen) {
        pen->onNewGrid(bookId,pageId,xIndex,yIndex,zIndex);
    }
}

void AiPenManager::onPenStrokeInterval(const std::string& mac, int strokeId, long long timeInterval)
{
    LOG(INFO) << "AiPenManager::onPenStrokeInterval pen=" << mac << ",strokeId=" << strokeId
              << ",interval=" << timeInterval;
}

void AiPenManager::onPenWordInterval(const std::string& mac, long long timeInterval)
{
    LOG(INFO) << "AiPenManager::onPenWordInterval pen=" << mac
              << ",interval=" << timeInterval;
}

void AiPenManager::onGetBookDetail(int result,const QJsonObject& js1, qulonglong tempData)
{
    //{"bookId":"B0007","characterStrokesNum":7,"characterXmlUpdateTime":"2021-10-22T05:56:39.000+0000",
    //"characterXmlUrl":"AI-pen/test/all/1933.xml","contentCode":1933,"contentId":405,"contentName":"沉",
    //"lessonId":66,"number":1,"page":33,"structureId":5}
    LOG(INFO) << "AiPenManager::onGetBookDetail result=" << result;
    qDebug() << "AiPenManager::onGetBookDetail,js1=" << js1;
    int* pBookId = reinterpret_cast<int*>(tempData);
    int bookId = *pBookId;
    delete pBookId;
    if(result == 0) {
        //请求成功,缓存数据
        //QJsonDocument::fromJson(bytes).object()
        QJsonArray data = js1["data"].toArray();
        if(data.size() > 0){
            int bookId = data.at(0).toObject()["bookId"].toString().mid(1).toInt();
            BookInfo bookInfo;
            for(int i = 0;i < data.size(); i++){
                QJsonObject json = data.at(i).toObject();
                int page = json["page"].toInt();
                int zIndex = json["number"].toInt();
                WordDetail wordDetail;
                wordDetail.contentCode = json["contentCode"].toInt();
                wordDetail.contentId = json["contentId"].toInt();
                wordDetail.contentName = json["contentName"].toString().toStdString();
                wordDetail.strokeNum = json["characterStrokesNum"].toInt();
                wordDetail.structId = json["structureId"].toInt();
                wordDetail.lessonId = json["lessonId"].toInt();
                wordDetail.characterXmlUrl = json["characterXmlUrl"].toString().toStdString();
                wordDetail.characterXmlUpdateTime = json["characterXmlUpdateTime"].toString().toStdString();
                emit updateWordXml(wordDetail.contentCode,QString::fromStdString(wordDetail.characterXmlUpdateTime),
                                   QString::fromStdString(wordDetail.characterXmlUrl),true);
                bookInfo.pageMap[page].insert(zIndex,wordDetail);
            }
            {
                QMutexLocker locker(&bookMutex_);
                bookInfoMap_.insert(bookId,bookInfo);
                requestingBooks_.remove(bookId);
            }

        } else {
            {
                QMutexLocker locker(&bookMutex_);
                requestingBooks_.remove(bookId);
                invalidBooks_.insert(bookId);
            }
        }
    } else {
        {
            QMutexLocker locker(&bookMutex_);
            //请求错误，清除正在检查的book
            requestingBooks_.clear();
        }
    }
    LOG(INFO) << "AiPenManager::onGetBookDetail end";
}


void AiPenManager::requestBookWordDetail(int bookId)
{

    {
        QMutexLocker locker(&bookMutex_);
        if(bookInfoMap_.contains(bookId) ||
                requestingBooks_.contains(bookId) ||
                invalidBooks_.contains(bookId)){
            LOG(INFO) << "AiPenManager::requestBookWordDetail already requested:" << bookId;
            return;
        }
    }
    std::string bookWordUrl = HttpConfig::instance()->getBookInfoDetail(bookId);

    LOG(INFO)<< "AiPenManager::requestBookWordDetail " << bookWordUrl;
    int* pBookId = new int(bookId);
    {
        QMutexLocker locker(&bookMutex_);
        requestingBooks_.insert(bookId);
    }
    emit httpRequest(QString::fromStdString(bookWordUrl),QByteArray(),1,MyRequestType::kRequestBookDetail,reinterpret_cast<qulonglong>(pBookId));

}

bool AiPenManager::invalidBookId(int bookId)
{
    LOG(INFO) << "AiPenManager::invalidBookId begin";
    bool invalidBook = false;
    {
        QMutexLocker locker(&bookMutex_);
        invalidBook = invalidBooks_.contains(bookId);
    }
    return invalidBook;
}


int AiPenManager::getWordDetail(int bookId, int pageId, int zIndex, WordDetail& detail)
{
    int wordCode = 0;
    bool needRequest = false;
    BookInfo* bookInfo = nullptr;
    {
        QMutexLocker locker(&bookMutex_);
        if(!bookInfoMap_.contains(bookId)) {
            if(!requestingBooks_.contains(bookId) && !invalidBooks_.contains(bookId)) {
                needRequest = true;
            }
        } else {
            bookInfo = &(bookInfoMap_[bookId]);
        }
    }
    if(needRequest) {
        requestBookWordDetail(bookId);
    }
    if(nullptr != bookInfo) {
        wordCode = bookInfo->getWordDetail(pageId,zIndex,detail);
    }
    return wordCode;
}

//wordIndex的序号是从1开始的
int AiPenManager::getWordDetail(int wordIndex, WordDetail& detail)
{
    if(practices_.empty()) {
        return 0;
    }

    int sum = 0;
    Practice tempPractice;
    bool found = false;
    for(auto practice : practices_) {
        sum += practice.times;
        if(sum >= wordIndex) {
            tempPractice = practice;
            found = true;
            break;
        }
    }

    if(!found) {
        return 0;
    }
    detail.contentCode = tempPractice.contentCode;
    detail.contentId = tempPractice.contentId;
    detail.contentName = tempPractice.contentName.toStdString();
    detail.strokeNum = tempPractice.characterStrokesNum;
    detail.structId = tempPractice.structureId;
    detail.wordOrder = tempPractice.wordOrder;
    detail.lessonId = lessonId_;
    return detail.contentCode;
}

void AiPenManager::handleDot(QString mac, int x, int fx, int y, int fy, int bookId, int pageId, int type, int force, long long timeStamp)
{
    if(showVerboseLog_) {
        LOG(INFO) << "AiPenManager::handleDot" << mac.toStdString() << ",x=" << x << "," << fx << "y=" << y << "," << fy
                  << ",note=" << bookId << ",page=" << pageId;
    }


    LOG(INFO) << "startAttended_ : " << startAttended_ << " startClasswork_:" << startClasswork_ ;

    if (startAttended_ || startClasswork_) {
        if(!startClasswork_) { //自由练习
            requestBookWordDetail(bookId);
        }
        AiPen* pPen = nullptr;

        LOG(INFO) << "AiPenManager::handleDot begin";
        bool penNotExists = false;
        {
            QMutexLocker locker(&penMapMutex_);
            if(!penHandleMap.contains(mac)) {
                LOG(INFO) << "!penHandleMap.contains(mac)  " << mac.toStdString();
                penNotExists = true;
            } else {
                pPen = penHandleMap[mac];
            }
        }
        LOG(INFO) << "AiPenManager::handleDot penNotExists = " << penNotExists ;
        if(penNotExists)
        {
            PenDataHandle* dataHandle = PenDataHandleFactory::createHandle(mac.toStdString());
            std::function<void(const std::string&)> func = AiPenManager::onPenClear;
            dataHandle->addOnClear(func);
            std::function<void(const std::string& ,int, int, int, int, int)> func2 = AiPenManager::onPenNewGrid;
            dataHandle->addOnNewGrid(func2);
            std::function<void(const std::string& ,int, std::vector<std::shared_ptr<Point>>&)> func3 = AiPenManager::onPenNewPoint;
            dataHandle->addOnNewPoint(func3);
            std::function<void(const std::string&)> func4 = AiPenManager::onPenDown;
            dataHandle->addOnPenDown(func4);
            std::function<void(const std::string&)> func5 = AiPenManager::onPenResult;
            dataHandle->addOnResult(func5);
            std::function<void(const std::string&,int,long long)> func6 = AiPenManager::onPenStrokeInterval;
            dataHandle->addOnStrokeTimeInterval(func6);
            std::function<void(const std::string&,long long)> func7 = AiPenManager::onPenWordInterval;
            dataHandle->addOnWordTimeInterval(func7);
            pPen = new AiPen(mac);

            pPen->setHandle(dataHandle);
            if(dictationStarted_) {
                QList<DictationWord*> tempWordList;
                {
                    QMutexLocker locker(&dictationWordListMutex_);
                    tempWordList = dictationWordList_;
                }
                for(auto word : tempWordList) {
                    pPen->handleNewDictationWord(word->groupId,
                                                 word->contentCodes);
                }

            }
            {
                QMutexLocker locker(&penMapMutex_);
                penHandleMap.insert(mac,pPen);
            }
        }

        float pointX = (float)x + ((float)fx / (float)100.0);
        float pointY = (float)y + ((float)fy / (float)100.0);
        if(nullptr != pPen) {
            pPen->dataHandle()->handleDot(pointX-2,pointY-2,bookId,pageId,type,force,timeStamp);
        }

//        emit newPoint(x*10,y*10);
    }
}

void AiPenManager::stopClassOnExit()
{
    startAttended_ = false;
    startClasswork_ = false;
    receivedHeartbeatPacket = false;
}

int AiPenManager::getWorkClassId(int lessonId)
{
    if(-1 == lessonId) {
        return kWCIInit; //无效lesson
    }
    int workClassId = kWCIInit;
    {
        QMutexLocker locker(&workClassIdMutex_);
        if(lessonIdToWorkClassId_.contains(lessonId)) {
            workClassId = lessonIdToWorkClassId_[lessonId];
        }
    }

     if(kWCIInit == workClassId) {
         {
             QMutexLocker locker(&workClassIdMutex_);
             lessonIdToWorkClassId_[lessonId] = kWCICreating;
         }
        std::string url = HttpConfig::instance()->getCreateWorkClassUrl(lessonId,curClassId_);
        emit httpRequest(QString::fromStdString(url),QByteArray(),HttpFuncType::kHttpGet,MyRequestType::kGetCreateWorkClass,qulonglong(lessonId));
        return kWCICreating;
    } else {
         return workClassId;
    }
}

void AiPenManager::onGetWorkClassId(int result, const QJsonObject &js1, qulonglong tempData)
{
    if(0 == result) {
        int lessonId = static_cast<int>(tempData);
        int workClassId = js1["data"].toInt();

        {
            QMutexLocker locker(&workClassIdMutex_);
            lessonIdToWorkClassId_[lessonId] = workClassId;
        }
    } else {
        int lessonId = static_cast<int>(tempData);
        {
            QMutexLocker locker(&workClassIdMutex_);
            lessonIdToWorkClassId_[lessonId] = kWCIInit;
        }
    }
}

void AiPenManager::finishLastClasswork()
{
    if(startClasswork_) {
        onNotifyStopClasswork();
    }
    if(dictationStarted_) {
        onNotifyStopDictation();
    }
}

