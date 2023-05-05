#include "messagehandler.h"

#include <QTimer>
#include <QPixmap>
#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QDir>

#include "logHelper.h"
//#include "aipenmanager.h"
#include "config.h"
#include "json/json.h"
#include "apmanager.h"
#include "commondefines.h"

using namespace Json;
using namespace std;

const QString kCommonConfigFileName("commonconfig.txt");

MessageHandler::MessageHandler(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<QList<QString>>("QList<QString>");
}

MessageHandler::~MessageHandler()
{
    LOG(INFO) <<"~MessageHandler()";
}

void MessageHandler::handleMessage(QString message)
{
    string stdJsonStr = message.toStdString();
    istringstream iss(stdJsonStr);
    CharReaderBuilder rBuilder;
    Value root;
    String err;
    bool ok = parseFromStream(rBuilder, iss, &root, &err);
    if (ok) {
        const char* cmd = root["command"].asCString();

        const char* id = root["id"].asCString();
        QString msgId = QString(id);

        if(strcmp(cmd, SaveToken) == 0) {
            handleSaveToken(msgId, root);
        }else if (strcmp(cmd, AddAp) == 0) {
            handleAddAp(msgId, root);
        }else if (strcmp(cmd, PenPair) == 0) {
            handlePenPair(msgId, root);
        }else if (strcmp(cmd, PenUnpair) == 0) {
            handlePenUnPair(msgId, root);
        }else if (strcmp(cmd, GetPenList) == 0) {
            handleGetPenList(msgId, root);
        }else if (strcmp(cmd, RegisterConnectStatusEvent) == 0) {
            handleRegisterAPConnectStatusEventListener(msgId);
        }else if (strcmp(cmd, UnregisterConnectStatusEvent) == 0) {
            handleUnregisterAPConnectStatusEventListener(msgId);
        }else if (strcmp(cmd, RegisterScanStatusEvent) == 0) {
            handleRegisterAPScanStatusEventListener(msgId);
        }else if (strcmp(cmd, UnregisterScanStatusEvent) == 0) {
            handleUnregisterAPScanStatusEventListener(msgId);
        }else if (strcmp(cmd, RegisterAPStatus) == 0) {
            handleRegisterAPStatusListener(msgId);
        }else if (strcmp(cmd, UnregisterAPStatus) == 0) {
            handleUnregisterAPStatusListener(msgId);
        }else if (strcmp(cmd, NotifyAttendClass) == 0) {
            handleNotifyAttendClass(msgId,root);
        }else if (strcmp(cmd, NotifyFinishClass) == 0) {
            handleNotifyFinishClass(msgId);
        }else if (strcmp(cmd, NotifyStartClasswork) == 0) {
            handleNotifyStartClasswork(msgId, root);
        }else if (strcmp(cmd, NotifyStopClasswork) == 0) {
            handleNotifyStopClasswork(msgId);
        }else if (strcmp(cmd, NotifyStartDictation) == 0) {
            handleNotifyStartDictation(msgId, root);
        }else if (strcmp(cmd, NotifyStopDictation) == 0) {
            handleNotifyStopDictation(msgId);
        }else if (strcmp(cmd, NotifyAccountManagePage) == 0) {
            handleNotifyAccountManagePage(msgId,root);
        }else if (strcmp(cmd, FrontendLog) == 0) {
            handleFrontendLog(root);
        }else if (strcmp(cmd, SaveImgToClipbrd) == 0){
            handleSaveImgToClipbrd(msgId, root);
        }else if (strcmp(cmd, WebSaveLoginInfo) == 0){
            handleWebSaveLoginInfo(msgId, root);
        }else if (strcmp(cmd, WebLoggout) == 0){
            handleWebLogout(msgId);
        }else if (strcmp(cmd, WebGetApMacs) == 0){
            handleWebGetApMacs(msgId);
        }else if (strcmp(cmd, WebSaveExerciseTimes) == 0){
            handleWebSaveExerciseTimes(msgId, root);
        }else if (strcmp(cmd, WebGetExerciseTimes) == 0){
            handleWebGetExerciseTimes(msgId);
        } else if (strcmp(cmd, RegisterUpdateState) == 0){
            handleRegisterUpdateStateListener(msgId);
        } else if (strcmp(cmd, UnregisterUpdateState) == 0){
            handleUnregisterUpdateStateListener(msgId);
        }else if (strcmp(cmd, RegisterPowerEvent) == 0){
            handleRegisterPowerEventListerner(msgId);
        }else if (strcmp(cmd, UnregisterPowerEvent) == 0){
            handleUnregisterPowerEventListerner(msgId);
        }else if (strcmp(cmd, PromptAboutDialog) == 0){
            handlePromptAboutDialog(msgId);
        }else if (strcmp(cmd, DictationWordStart) == 0){
            handleDictationWordStart(msgId, root);
        }else if (strcmp(cmd, GetRecoveryInfo) == 0){
            handleGetRecoveryInfo(msgId);
        }else if (strcmp(cmd, GetCommonConfig) == 0){
            handleGetCommonConfig(msgId);
        }else if (strcmp(cmd, SaveCommonConfig) == 0){
            handleSaveCommonConfig(msgId,root);
        } else if(strcmp(cmd,"closeMainWindow") == 0) {
            LOG(INFO) << "electron: closeMainWindow";
            emit closeMainWindow();
        } else {
            emit sendTextMessage(buildNoDataResponse(msgId, cmd, 500, "unsupported command."));
            LOG(WARNING) << "Unsupported command: "<< cmd;
        }
    }
    else {
        LOG(INFO) << err.c_str() <<endl;
    }
}

void MessageHandler::handleSaveToken(const QString& msgId, const Value& value)
{
    Json::Value body = value["body"];
    if (body.isMember("token")){
        QString token = QString(value["body"]["token"].asCString());
        Config::saveToken(token);
        emit saveToken(token);
        emit sendTextMessage(buildNoDataResponse(msgId, SaveToken, 0, "success"));
    }else {
        emit sendTextMessage(buildNoDataResponse(msgId, SaveToken, 500, "no token"));
    }
}

void MessageHandler::handleAddAp(const QString& msgId, const Json::Value &value)
{
    webMsgIds_[QString(AddAp)] = msgId;
    QList<QString> ipOrMacList;
    Json::Value rawDeviceList = value["body"]["ApList"];
    for (Json::Value::ArrayIndex i = 0; i < rawDeviceList.size(); i++){
        ipOrMacList.append(QString(rawDeviceList[i].asCString()));
    }
    emit addAp(msgId,ipOrMacList);
}

void MessageHandler::handlePenPair(const QString& msgId, const Json::Value &value)
{
    QList<QString> macList;
    Json::Value rawMacList = value["body"]["macList"];
    for (unsigned int i = 0; i < rawMacList.size(); ++i)
    {
        macList.append(QString(rawMacList[i].asCString()));
    }
    emit penPair(msgId, macList);
}

void MessageHandler::handlePenUnPair(const QString& msgId, const Json::Value &value)
{
    QList<QString> macList;
    Json::Value rawMacList = value["body"]["macList"];
    for (unsigned int i = 0; i < rawMacList.size(); ++i)
    {
        macList.append(QString(rawMacList[i].asCString()));
    }
    emit penUnPair(msgId, macList);
}

void MessageHandler::handleGetPenList(const QString& msgId, const Json::Value &value)
{
    QList<QString> macList;
    Json::Value rawMacList = value["body"]["macList"];
    for (unsigned int i = 0; i < rawMacList.size(); ++i)
    {
        macList.append(QString(rawMacList[i].asCString()));
    }
    emit getPenList(msgId, macList);
}

void MessageHandler::handleRegisterAPConnectStatusEventListener(const QString& msgId)
{
    webMsgIds_[QString(RegisterConnectStatusEvent)] = msgId;
    emit sendTextMessage( buildNoDataResponse(msgId, RegisterConnectStatusEvent, 0, "success"));
}

void MessageHandler::handleUnregisterAPConnectStatusEventListener(const QString& msgId)
{
    webMsgIds_.remove(QString(RegisterConnectStatusEvent));
    emit sendTextMessage(buildNoDataResponse(msgId, UnregisterConnectStatusEvent, 0, "success"));
}

void MessageHandler::handleRegisterAPScanStatusEventListener(const QString& msgId)
{
    webMsgIds_[QString(RegisterScanStatusEvent)] = msgId;
    emit sendTextMessage(buildNoDataResponse(msgId, RegisterScanStatusEvent, 0, "success"));
}

void MessageHandler::handleUnregisterAPScanStatusEventListener(const QString& msgId)
{
    webMsgIds_.remove(QString(RegisterScanStatusEvent));
    emit sendTextMessage(buildNoDataResponse(msgId, UnregisterScanStatusEvent, 0, "success"));
}

void MessageHandler::handleRegisterAPStatusListener(const QString& msgId)
{
    webMsgIds_[QString(RegisterAPStatus)] = msgId;
    emit sendTextMessage(buildNoDataResponse(msgId, RegisterAPStatus, 0, "success"));
}

void MessageHandler::handleUnregisterAPStatusListener(const QString& msgId)
{
    webMsgIds_.remove(QString(RegisterAPStatus));
    emit sendTextMessage(buildNoDataResponse(msgId, UnregisterAPStatus, 0, "success"));
}

void MessageHandler::handleNotifyAttendClass(const QString& msgId, const Json::Value& value)
{
    Json::Value body = value["body"];
    if (body.isMember("curClassId")){
        emit notifyAttendClass(body["curClassId"].asInt());
        QTimer *timer = new QTimer();
        counter = 1 * 60;
        connect(timer, &QTimer::timeout, [msgId, this, timer](){
            counter--;
            if(Config::GetHeartbeat()){ //ap的心跳
                emit sendTextMessage(buildNoDataResponse(msgId, NotifyAttendClass, 0 , "success"));
                timer->stop();
                timer->deleteLater();
            } else if (counter == 0) {
                emit sendTextMessage(buildNoDataResponse(msgId, NotifyAttendClass, 500 ,
                                                                        "no received next heartbeat packet, maybe ap disconnected."));
                timer->stop();
                timer->deleteLater();
            } else {
                LOG(INFO)<<"no received next heartbeat packet.";
            }
        });
        timer->start(1000);
    } else {
        emit sendTextMessage(buildNoDataResponse(msgId, NotifyAttendClass, 500 ,
                                                                "No class id."));
    }
}

void MessageHandler::handleNotifyFinishClass(const QString& msgId)
{
    QTimer *timer = new QTimer();
    connect(timer, &QTimer::timeout, [msgId, this, timer](){
        emit notifyFinishClass();
        emit sendTextMessage(buildNoDataResponse(msgId, NotifyFinishClass, 0 , "success"));
        timer->stop();
        timer->deleteLater();
    });
    timer->start(5*1000);

}

void MessageHandler::handleNotifyStartClasswork(const QString& msgId, const Json::Value &value)
{
#if 0
    Classwork* work = new Classwork();
    work->workClassId = value["body"]["workClassId"].asInt();
    work->endTime = value["body"]["endTime"].asLargestUInt();
    work->lessonId = value["body"]["lessonId"].asInt();
    work->practiseType = value["body"]["practiseType"].asInt();
    work->practiseBriefInfo = QString::fromStdString(value["body"]["practiseBriefInfo"].asCString());
    work->practices.clear();
    QDateTime time = QDateTime::currentDateTime();//获取当前时间
    work->startTime = time.toMSecsSinceEpoch();
    Value rawWordsList = value["body"]["words"];
    int rowIndex = 0;
    for (unsigned int i = 0; i < rawWordsList.size(); ++i)
    {
        //todo
        Value word = rawWordsList[i];
        Practice p;
        p.times = word["times"].asLargestUInt();
        p.contentCode = word["contentCode"].asInt();
        QString contentName = QString::fromStdString(word["contentName"].asCString());
        auto name = QByteArray::fromPercentEncoding(contentName.toLatin1());
        p.contentName = QString::fromUtf8(name);

        int wordOrder = -1;
        if(word.isMember("wordOrder")) {
            wordOrder = word["wordOrder"].asInt();
        }
        int row = -1;
        if(word.isMember("row")) {
            row = word["row"].asInt();
        }

        if(-1 != wordOrder && -1 == row) { //有wordOrder，但是row还没添加
            if (0 == wordOrder) {
                ++rowIndex; //row从1开始，不同于wordOrder
            }
            row = rowIndex;
        }

        if(-1 != wordOrder && -1 != row) {
            p.wordOrder = row * 10000 + wordOrder; //跟APP端一致
        } else {
            p.wordOrder = -1; //无效标记
        }

        if(0 == p.contentCode) {
            p.contentCode = -1;
        } else {
            p.contentId = word["contentId"].asInt();
            p.structureId = word["structureId"].asInt();
            p.characterStrokesNum = word["characterStrokesNum"].asInt();
            p.characterXmlUrl = QString::fromStdString(word["characterXmlUrl"].asCString());
            if(word.isMember("characterXmlUpdateTime")) {
                p.characterXmlUpdateTime = QString::fromStdString(word["characterXmlUpdateTime"].asCString());
            }
        }
        work->practices.push_back(p);
    }
#endif
    StreamWriterBuilder wBuilder;
    emit notifyStartClasswork(QString(writeString(wBuilder, value).c_str()));
    emit sendTextMessage(buildNoDataResponse(msgId, NotifyStartClasswork, 0 , "success"));
}

void MessageHandler::handleNotifyStopClasswork(const QString& msgId)
{
    emit notifyStopClasswork();
    QTimer *timer = new QTimer();
    connect(timer, &QTimer::timeout, [msgId, this, timer](){
        emit sendTextMessage(buildNoDataResponse(msgId, NotifyStopClasswork, 0 , "success"));
        timer->stop();
        timer->deleteLater();
    });
    timer->start(1*1000);
}

void MessageHandler::handleNotifyStartDictation(const QString &msgId, const Value &value)
{
    int workClassId = value["body"]["workClassId"].asInt();
    emit notifyStartDictation(workClassId);
    emit sendTextMessage(buildNoDataResponse(msgId, NotifyStartDictation, 0 , "success"));
}

void MessageHandler::handleNotifyStopDictation(const QString &msgId)
{
    emit notifyStopDictation();
    QTimer *timer = new QTimer();
    connect(timer, &QTimer::timeout, [msgId, this, timer](){
        emit sendTextMessage(buildNoDataResponse(msgId, NotifyStopDictation, 0 , "success"));
        timer->stop();
        timer->deleteLater();
    });
    timer->start(3*1000);
}

void MessageHandler::handleNotifyAccountManagePage(const QString& msgId, const Json::Value &value)
{
    QString url = value["body"]["url"].asCString();
    emit notifyAccountManagePage(url);
    emit sendTextMessage(buildNoDataResponse(msgId, NotifyAccountManagePage, 0 , "success"));
}

void MessageHandler::handleFrontendLog(const Json::Value &value)
{
    const char* log = value["body"]["log"].asCString();
    LOG(INFO)<<"FrontendLog:"<<log;
}

void MessageHandler::handleSaveImgToClipbrd(const QString& msgId, const Json::Value &value)
{
    QString img = value["body"]["img"].asCString();
    QPixmap *image = new QPixmap;
    image->loadFromData(QByteArray::fromBase64(img.toLocal8Bit()));
    emit copyImgToClipboard((qulonglong)image);
    emit sendTextMessage(buildNoDataResponse(msgId, SaveImgToClipbrd, 0 , "success"));
}

void MessageHandler::handleWebSaveLoginInfo(const QString& msgId, const Json::Value &value)
{
    quint64 userId = 0;
    QString token;
    bool bAutoLogin = false;
    Json::Value body = value["body"];
    if(body.isMember("userId")) {
        userId = value["body"]["userId"].asInt64();
    }
    if(body.isMember("token")) {
        token = value["body"]["token"].asCString();
    }

    if (body.isMember("autoLogin"))
    {
        bAutoLogin = value["body"]["autoLogin"].asBool();
    }

    QString loginId = value["body"]["loginId"].asCString();
    Config::saveLoginInfo(userId,token,loginId, bAutoLogin);
    emit sendTextMessage(buildNoDataResponse(msgId, WebSaveLoginInfo, 0 , "success"));
}

void MessageHandler::handleWebLogout(const QString& msgId)
{
    Config::removeLoginInfo();
    emit sendTextMessage(buildNoDataResponse(msgId, WebLoggout, 0 , "success"));
}

void MessageHandler::handleWebGetApMacs(const QString& msgId)
{
    QStringList macs = Config::getMacList();

    StreamWriterBuilder wBuilder;
    Value root;
    root["id"] = msgId.toStdString();
    root["command"] = WebGetApMacs;
    Value body;
    body["code"] = 0;
    body["msg"] = "success";

    Json::Value macArray;
    if(macs.empty()) {
        macArray.resize(0);
    } else {
        for(QString mac : macs) {
            macArray.append(mac.toStdString());
        }
    }
    body["data"] = macArray;
    root["body"] = body;

    QString content(writeString(wBuilder, root).c_str());

    emit sendTextMessage( content);
}

void MessageHandler::handleWebSaveExerciseTimes(const QString &msgId, const Value &value)
{
    Json::Value body = value["body"];
    Value data = body["data"];
    QStringList times;
    unsigned dataSize = data.size();
    for(unsigned i = 0; i < dataSize; ++i) {
        times << QString::number(data[i].asInt());
    }
    Config::saveExerciseTimes(times.join('&'));
    emit sendTextMessage(buildNoDataResponse(msgId, WebSaveExerciseTimes, 0 , "success"));
}

void MessageHandler::handleWebGetExerciseTimes(const QString &msgId)
{
    QString timesStr = Config::getExerciseTimes();

    QStringList timesList;

    if(!timesStr.isEmpty()) {
        timesList = timesStr.split('&');
    }

    StreamWriterBuilder wBuilder;
    Value root;
    root["id"] = msgId.toStdString();
    root["command"] = WebGetExerciseTimes;
    Value body;
    body["code"] = 0;
    body["msg"] = "success";

    Json::Value timesArray;
    if(timesList.empty()) {
        timesArray.resize(0);
    } else {
        for(QString times : timesList) {
            timesArray.append(times.toInt());
        }
    }
    body["data"] = timesArray;
    root["body"] = body;

    QString content(writeString(wBuilder, root).c_str());

    emit sendTextMessage(content);
}

void MessageHandler::handlePromptAboutDialog(const QString& msgId)
{
//    emit promptAboutDialog();
    emit sendTextMessage(buildNoDataResponse(msgId, PromptAboutDialog, 0, "success"));
}

void MessageHandler::handleDictationWordStart(const QString &msgId, const Value &value)
{
//    DictationWord* wordInfo = new DictationWord;
//    wordInfo->groupId = value["body"]["groupWord"]["groupId"].asInt();
//    QString contentCode = value["body"]["groupWord"]["contentCode"].asCString();

//    QStringList contentCodeList = contentCode.split(",");
//    for(QString code : contentCodeList) {
//        wordInfo->contentCodes.push_back(code.toInt());
//    }
    emit sendTextMessage(buildNoDataResponse(msgId, DictationWordStart, 0 , "success"));
    Value obj;
    obj["groupId"] = value["body"]["groupWord"]["groupId"].asInt();
    obj["contentCode"] = value["body"]["groupWord"]["contentCode"].asCString();

    StreamWriterBuilder wBuilder;
    emit dictationWordStart(QString(writeString(wBuilder, obj).c_str()));
}

void MessageHandler::handleRegisterUpdateStateListener(const QString& msgId)
{
    webMsgIds_[QString(RegisterUpdateState)] = msgId;
    emit sendTextMessage(buildNoDataResponse(msgId, RegisterUpdateState, 0, "success"));
}

void MessageHandler::handleUnregisterUpdateStateListener(const QString& msgId)
{
    webMsgIds_.remove(QString(RegisterUpdateState));
    emit sendTextMessage(buildNoDataResponse(msgId, UnregisterUpdateState, 0, "success"));
}

void MessageHandler::handleRegisterPowerEventListerner(const QString& msgId)
{
    webMsgIds_[QString(RegisterPowerEvent)] = msgId;
    emit sendTextMessage(buildNoDataResponse(msgId, RegisterPowerEvent, 0, "success"));
}

void MessageHandler::handleUnregisterPowerEventListerner(const QString& msgId)
{
    webMsgIds_.remove(QString(RegisterPowerEvent));
    emit sendTextMessage(buildNoDataResponse(msgId, UnregisterPowerEvent, 0, "success"));
}

void MessageHandler::handleGetRecoveryInfo(const QString &msgId)
{
    emit sendTextMessage(buildGetRecoveryResponse(msgId, 0 , ""));
}

QString MessageHandler::buildGetRecoveryResponse(QString id, int code, string msg)
{
    StreamWriterBuilder wBuilder;
    Value root;
    root["id"] = id.toStdString();
    root["command"] = GetRecoveryInfo;
    Value body;
    body["code"] = code;
    body["msg"] = msg;

    Value data;
    static bool bIsFirst = true;
    if (bIsFirst)
    {
        bIsFirst = false;
        bool bIsBeginClass = false;
        int nCurClassId = 1;
        bool bIsBeginClasswork = false;
        int nWorkClassId = 1;

        Config::getClassInfo(bIsBeginClass, nCurClassId);
        Config::getClassWorkInfo(bIsBeginClasswork, nWorkClassId);

        data["curClassId"] = nCurClassId;
        data["workClassId"] = nWorkClassId;
    }

    body["data"] = data;
    root["body"] = body;
    QString jsonMsg = QString(writeString(wBuilder, root).c_str());
    LOG(INFO) << "MessageHandler buildGetRecoveryResponse jsonMsg = " <<jsonMsg.toStdString();
    return jsonMsg;
}

void MessageHandler::handleGetCommonConfig(const QString &msgId)
{
    return sendTextMessage(buildGetCommmonConfig(msgId));
}

void MessageHandler::handleSaveCommonConfig(const QString &msgId, const Value &value)
{
    Json::Value body = value["body"];
    Value data = body["data"];
    QStringList times;
    unsigned dataSize = data.size();
    for(unsigned i = 0; i < dataSize; ++i) {
        times << QString::number(data[i].asInt());
    }
    Config::saveExerciseTimes(times.join('&'));

    QDir dir(Config::getAipenPath());
    QFile file(dir.absoluteFilePath(kCommonConfigFileName));
    if(file.open(QFile::WriteOnly)) {
        QByteArray writeData(data.asCString());
        file.write(writeData);
        file.close();
    }

    emit sendTextMessage(buildNoDataResponse(msgId, SaveCommonConfig, 0 , "success"));
}

QString MessageHandler::buildGetCommmonConfig(QString id)
{
    StreamWriterBuilder wBuilder;
    Value root;
    root["id"] = id.toStdString();
    root["command"] = GetCommonConfig;
    Value body;

    QString data;
    QDir dir(Config::getAipenPath());
    if(dir.exists(kCommonConfigFileName)) {
        QFile file(dir.absoluteFilePath(kCommonConfigFileName));
        if(file.open(QFile::ReadOnly | QFile::Text)) {
            data = file.readAll();
            file.close();
        }
    }

    if(data.isEmpty()) {
        body["code"] = 1;
        body["msg"] = "no config";
    } else {
        body["code"] = 0;
        body["msg"] = "";
    }

    body["data"] = data.toStdString();
    root["body"] = body;
    QString jsonMsg = QString(writeString(wBuilder, root).c_str());
    LOG(INFO) << "MessageHandler buildGetCommmonConfig jsonMsg = " <<jsonMsg.toStdString();
    return jsonMsg;
}

QString MessageHandler::buildNoDataResponse(QString id, std::string cmd, int code, std::string msg)
{
    StreamWriterBuilder wBuilder;
    Value root;
    root["id"] = id.toStdString();
    root["command"] = cmd;
    Value body;
    body["code"] = code;
    body["msg"] = msg;
    root["body"] = body;
    return QString(writeString(wBuilder, root).c_str());
}
