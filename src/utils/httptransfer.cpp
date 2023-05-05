/*
* Copyright (c) 2020, 深圳市优必选科技有限公司
* All rights reserved.
*
* 文件名称：httptransfer.h
* 创建时间：2020/06/22
* 文件标识：
* 文件摘要：http传输类，完成http指令请求和数据的获取
*
* 当前版本：1.0.0.0
* 作    者：Joker
* 完成时间：2020/06/22
* 版本摘要：
*/

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
//#include <QApplication>
#include <QDir>
#include <QUuid>
#include <string>

#include "httptransfer.h"

HttpTransfer::HttpTransfer(ILibFileSynchronize* pFileSyn):
    m_pFileSyn(pFileSyn)
  , m_strFileData("")
{
    m_pNetManager = NULL;
    connect(this, &HttpTransfer::SigPostQueryFileList, this, &HttpTransfer::OnPostPushFile);
    connect(this, &HttpTransfer::SigGetQueryFileList, this, &HttpTransfer::OnGetFileList);
    connect(this, &HttpTransfer::SigGetQueryAlyOssKey, this, &HttpTransfer::OnGetQueryAlyOssKey);
    connect(this, &HttpTransfer::SigGetQueryOTAVersion, this, &HttpTransfer::OnGetQueryOTAVersion);
    connect(this, &HttpTransfer::SigGetUCodeToken, this, &HttpTransfer::onTokenToUCodeToken);
    connect(this, &HttpTransfer::SigInit, this, &HttpTransfer::OnInit);
    moveToThread(&m_oThread);
    m_nRequestID = -1;
    m_oThread.start();
    emit SigInit();
}

HttpTransfer::~HttpTransfer()
{
    m_oThread.quit();
    if(m_pNetManager)
    {
        m_pNetManager->disconnect();
        delete  m_pNetManager;
        m_pNetManager = NULL;
        //        SAFE_DELETE(m_pNetManager); // 出现释放打印异常
    }
}

/************************************
* 名称: stopThread
* 功能: 退出线程
* 参数:
* 返回:
* 时间:   2020/04/06
* 作者:   Joker
************************************/
void HttpTransfer::stopThread()
{
    m_oThread.quit();
    m_oThread.wait();
}

/************************************
* 名称: postQueryFileList
* 功能: post http请求，内部封装了多线程处理
* 参数: [in]serverURL 请求资源地，
* 参数: [in]data 请求参数数据
* 参数: [in]nOldRequestID 上次请求的ID，若还没完成，可以结束上次请求
* 返回:   quint32 返回请求ID,外部监听信号时用于判断是否是自己需要的消息
* 时间:   2020/03/24
* 作者:   Joker
************************************/
int HttpTransfer::postQueryFileList(const QString &serverURL, const QString &data, int nOldRequestID)
{
    if(!m_oThread.isRunning())
        m_oThread.start();

    //先停止旧的请求，如果存在
    if(m_mapReplyFileLst.contains(nOldRequestID))
    {
        m_mapReplyFileLst[nOldRequestID]->abort();
    }

    int nRequestID = createUnusedRequestID();
    emit SigPostQueryFileList(nRequestID, serverURL, data);
    return nRequestID;
}

int HttpTransfer::getQueryFileList(const QString &serverURL, int nOldRequestID, int nSyncType)
{
    if(!m_oThread.isRunning())
        m_oThread.start();

    //先停止旧的请求，如果存在
    if(m_mapReplyFileLst.contains(nOldRequestID))
    {
        m_mapReplyFileLst[nOldRequestID]->abort();
    }

    int nRequestID = createUnusedRequestID();
    emit SigGetQueryFileList(nRequestID, serverURL, nSyncType);
    return nRequestID;
}

int HttpTransfer::getQueryAlyOssKey(const QString &serverURL, int nOldRequestID)
{
    if(!m_oThread.isRunning())
        m_oThread.start();

    //先停止旧的请求，如果存在
    if(m_mapReplyFileLst.contains(nOldRequestID))
    {
        m_mapReplyFileLst[nOldRequestID]->abort();
    }
    UBTUtil::print_msg("HttpTransfer::getQueryAlyOssKey serverURL=%s\n",serverURL.toUtf8().data());
    int nRequestID = createUnusedRequestID();
    emit SigGetQueryAlyOssKey(nRequestID, serverURL);
    return nRequestID;
}

/************************************
* 名称: getQueryOTAVersion
* 功能: 查询OTA版本
* 参数: [in]serverURL 请求资源地，
* 参数: [in]nOldRequestID 上次请求的ID，若还没完成，可以结束上次请求
* 参数: [in]bReply 收到结果后是否需要回复web
* 返回:   int 返回请求ID,外部监听信号时用于判断是否是自己需要的消息
* 时间:   2020/03/24
* 作者:   Joker
************************************/
int HttpTransfer::getQueryOTAVersion(const QString &serverURL, int nOldRequestID, bool bReply)
{
    if(!m_oThread.isRunning())
        m_oThread.start();

    //先停止旧的请求，如果存在
    if(m_mapReplyFileLst.contains(nOldRequestID))
    {
        m_mapReplyFileLst[nOldRequestID]->abort();
    }

    int nRequestID = createUnusedRequestID();
    emit SigGetQueryOTAVersion(nRequestID, serverURL, bReply);
    return nRequestID;
}

QString HttpTransfer::getUCodeToken(const QString &serverURL, int nOldRequestID)
{
    if(!m_oThread.isRunning())
        m_oThread.start();

    //先停止旧的请求，如果存在
    if(m_mapReplyFileLst.contains(nOldRequestID))
    {
        m_mapReplyFileLst[nOldRequestID]->abort();
    }

    int nRequestID = createUnusedRequestID();

    emit SigGetUCodeToken(nRequestID, serverURL);

    return "test token";
}

void HttpTransfer::resetResp()
{
    m_bytesResponse.clear();
}

int HttpTransfer::createUnusedRequestID()
{
    QMutexLocker lock(&m_mutex);
    ++m_nRequestID;
    return m_nRequestID;
}

/************************************
* 名称: OnPostPushFile
* 功能: post http请求槽函数
* 参数: [in]nNewRequestID 新建的请求ID
* 参数: [in]serverURL 请求资源地，
* 参数: [in]data 请求参数数据
* 返回:   int 返回请求ID,外部监听信号时用于判断是否是自己需要的消息
* 时间:   2020/03/24
* 作者:   Joker
************************************/
void HttpTransfer::OnPostPushFile(int nNewRequestID, const QString &serverURL, const QString &data)
{
    m_mapParamData.insert(nNewRequestID, data.toUtf8());
    QNetworkRequest request;
    QUrl url(serverURL);
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    UBTServerConfig config;
    QString strUBTSign = config.getHeaderUBTSign();

    qDebug()<< "HttpTransfer::OnPostPushFile strUBTSign =  "<< strUBTSign;
    QString strSource =config.getLearnLessonSource();
    qDebug()<< "HttpTransfer::OnPostPushFile strUBTSign =  "<< strSource;
    //    request.setHeader(QNetworkRequest::ContentLengthHeader, m_mapParamData[nNewRequestID].length());
    QByteArray arUBTSign = strUBTSign.toUtf8();
    QByteArray arSource = strSource.toUtf8();
    request.setRawHeader("X-UBT-Sign", arUBTSign);
    request.setRawHeader("X-UBT-Source", arSource);

    //通用认证部分
    QString strAppId =  config.getLearnLessonAppId();
    QString strDeviceId =  config.getLearnLessonDeviceID();
    QString strLanguage =  config.getLearnLessonLanguage();

    QString Token = CIniConfig::getInstance().getValueToken();
    request.setRawHeader("X-UBT-AppId", strAppId.toUtf8());
    request.setRawHeader("X-UBT-DeviceId", strDeviceId.toUtf8());
    request.setRawHeader("X-UBT-language", strLanguage.toUtf8());
    request.setRawHeader("authorization", Token.toUtf8());


    request.setAttribute(QNetworkRequest::Attribute(Constants::HTTPRequestID),qVariantFromValue((int)nNewRequestID));//设置ID
    request.setAttribute(QNetworkRequest::Attribute(Constants::HTTPRequestType), Constants::HTTP_REQUEST_PUSH_FILE);//设置类型
    QNetworkReply* reply = m_pNetManager->post(request, m_mapParamData[nNewRequestID]);
    connect(reply, &QNetworkReply::finished, this, &HttpTransfer::OnFinished);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
            this, &HttpTransfer::OnError);
    connect(reply, &QNetworkReply::downloadProgress,
            this, &HttpTransfer::OnDownloadProgress);
    connect(reply, &QNetworkReply::readyRead,
            this, &HttpTransfer::readyRead);
    m_mapReplyFileLst.insert(nNewRequestID, reply);

}

void HttpTransfer::OnGetFileList(int nNewRequestID, const QString &serverURL, int nSyncType)
{
    QNetworkRequest request;
    QUrl url(serverURL);
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");


    UBTServerConfig config;
    QString strUBTSign = config.getHeaderUBTSign();

    qDebug()<< "HttpTransfer::OnPostPushFile strUBTSign =  "<< strUBTSign;
    QString strSource =config.getLearnLessonSource();
    qDebug()<< "HttpTransfer::OnPostPushFile strUBTSign =  "<< strSource;
    //    request.setHeader(QNetworkRequest::ContentLengthHeader, m_mapParamData[nNewRequestID].length());
    QByteArray arUBTSign = strUBTSign.toUtf8();
    QByteArray arSource = strSource.toUtf8();
    request.setRawHeader("X-UBT-Sign", arUBTSign);
    request.setRawHeader("X-UBT-Source", arSource);

    //通用认证部分
    QString strAppId =  config.getLearnLessonAppId();
    QString strDeviceId =  config.getLearnLessonDeviceID();
    QString strLanguage =  config.getLearnLessonLanguage();
    QString Token = CIniConfig::getInstance().getValueToken();
    request.setRawHeader("X-UBT-AppId", strAppId.toUtf8());
    request.setRawHeader("X-UBT-DeviceId", strDeviceId.toUtf8());
    request.setRawHeader("X-UBT-language", strLanguage.toUtf8());
    request.setRawHeader("authorization", Token.toUtf8());


    request.setAttribute(QNetworkRequest::Attribute(Constants::HTTPRequestID),qVariantFromValue((int)nNewRequestID));//设置ID
    request.setAttribute(QNetworkRequest::Attribute(Constants::HTTPRequestType), Constants::HTTP_REQUEST_GET_FILE_LIST);//设置类型
    request.setAttribute(QNetworkRequest::Attribute(Constants::HTTPFlieSyncType), (int)nSyncType);//设置类型

    //    qDebug()<< " HttpTransfer::OnGetFileList"<<reque;
    QNetworkReply* reply = m_pNetManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &HttpTransfer::OnFinished);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
            this, &HttpTransfer::OnError);
    connect(reply, &QNetworkReply::downloadProgress,
            this, &HttpTransfer::OnDownloadProgress);
    connect(reply, &QNetworkReply::readyRead,
            this, &HttpTransfer::readyRead);
    m_mapReplyFileLst.insert(nNewRequestID, reply);
}

void HttpTransfer::OnGetQueryAlyOssKey(int nNewRequestID, const QString &serverURL)
{
    QNetworkRequest request;
    QUrl url(serverURL);
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");


    UBTServerConfig config;
    QString strUBTSign = config.getHeaderUBTSign();

    qDebug()<< "HttpTransfer::OnPostPushFile strUBTSign =  "<< strUBTSign;
    QString strSource =config.getLearnLessonSource();
    qDebug()<< "HttpTransfer::OnPostPushFile strUBTSign =  "<< strSource;
    //    request.setHeader(QNetworkRequest::ContentLengthHeader, m_mapParamData[nNewRequestID].length());
    QByteArray arUBTSign = strUBTSign.toUtf8();
    QByteArray arSource = strSource.toUtf8();
    request.setRawHeader("X-UBT-Sign", arUBTSign);
    request.setRawHeader("X-UBT-Source", arSource);

    //通用认证部分
    QString strAppId =  config.getLearnLessonAppId();
    QString strDeviceId =  config.getLearnLessonDeviceID();
    QString strLanguage =  config.getLearnLessonLanguage();
    QString Token = CIniConfig::getInstance().getValueToken();
    request.setRawHeader("X-UBT-AppId", strAppId.toUtf8());
    request.setRawHeader("X-UBT-DeviceId", strDeviceId.toUtf8());
    request.setRawHeader("X-UBT-language", strLanguage.toUtf8());
    request.setRawHeader("authorization", Token.toUtf8());

    UBTUtil::print_msg("HttpTransfer::OnGetQueryAlyOssKey nNewRequestID = %d\n", nNewRequestID);
    request.setAttribute(QNetworkRequest::Attribute(Constants::HTTPRequestID),qVariantFromValue((int)nNewRequestID));//设置ID
    request.setAttribute(QNetworkRequest::Attribute(Constants::HTTPRequestType), Constants::HTTP_REQUEST_ALY_KEY);//设置类型
    QNetworkReply* reply = m_pNetManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &HttpTransfer::OnFinished);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
            this, &HttpTransfer::OnError);
    connect(reply, &QNetworkReply::downloadProgress,
            this, &HttpTransfer::OnDownloadProgress);
    connect(reply, &QNetworkReply::readyRead,
            this, &HttpTransfer::readyRead);
    m_mapReplyFileLst.insert(nNewRequestID, reply);
}

/************************************
* 名称: SigGetQueryOTAVersion
* 功能: 查询OTA版本
* 参数: [in]nNewRequestID 上次请求的ID，若还没完成，可以结束上次请求
* 参数: [in]serverURL 请求资源地，
* 参数: [in]bReply 收到结果后是否需要回复web
* 返回:   int 返回请求ID,外部监听信号时用于判断是否是自己需要的消息
* 时间:   2020/03/24
* 作者:   Joker
************************************/
void HttpTransfer::OnGetQueryOTAVersion(int nNewRequestID, const QString &serverURL, bool bReply)
{
    QNetworkRequest request;
    QUrl url(serverURL);
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");


    UBTServerConfig config;
    QString strUBTSign = config.getHeaderUBTSign();

    qDebug()<< "HttpTransfer::OnPostPushFile strUBTSign =  "<< strUBTSign;
    //    request.setHeader(QNetworkRequest::ContentLengthHeader, m_mapParamData[nNewRequestID].length());
    QByteArray arUBTSign = strUBTSign.toUtf8();
    request.setRawHeader("X-UBT-Sign", arUBTSign);
    //通用认证部分
    QString strAppId =  config.getLearnLessonAppId();
    QString strDeviceId =  config.getLearnLessonDeviceID();
    //    QString strLanguage =  config.getLearnLessonLanguage();
    //    QString Token = CIniConfig::getInstance().getValueToken();
    request.setRawHeader("X-UBT-AppId", strAppId.toUtf8());
    request.setRawHeader("X-UBT-DeviceId", strDeviceId.toUtf8());
    //    request.setRawHeader("X-UBT-language", strLanguage.toUtf8());
    //    request.setRawHeader("authorization", Token.toUtf8());


    request.setAttribute(QNetworkRequest::Attribute(Constants::HTTPRequestID),qVariantFromValue((int)nNewRequestID));//设置ID


    request.setAttribute(QNetworkRequest::Attribute(Constants::HTTPRequestType), Constants::HTTP_REQUEST_GET_OTA_VERSION);//设置类型
    request.setAttribute(QNetworkRequest::Attribute(Constants::HTTPOtaNeedReply), bReply);//设置类型
    QNetworkReply* reply = m_pNetManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &HttpTransfer::OnFinished);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
            this, &HttpTransfer::OnError);
    connect(reply, &QNetworkReply::downloadProgress,
            this, &HttpTransfer::OnDownloadProgress);
    connect(reply, &QNetworkReply::readyRead,
            this, &HttpTransfer::readyRead);
    m_mapReplyFileLst.insert(nNewRequestID, reply);
}

void HttpTransfer::onTokenToUCodeToken(int nNewRequestID, const QString &serverURL)
{
    QNetworkRequest request;
    QUrl url(serverURL);
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    UBTServerConfig config;
    QString strUBTSign = config.getHeaderUBTSign();

    qDebug()<< "HttpTransfer::tokenToUCodeToken strUBTSign =  "<< strUBTSign;

    QByteArray arUBTSign = strUBTSign.toUtf8();
    request.setRawHeader("X-UBT-Sign", arUBTSign);
    request.setRawHeader("X-UBT-Source", "eduteacher");

    //通用认证部分
    QString strAppId =  config.getLearnLessonAppId();
    QString strDeviceId =  config.getLearnLessonDeviceID();
    QString strLanguage =  config.getLearnLessonLanguage();
    QString Token = CIniConfig::getInstance().getValueToken();
    request.setRawHeader("X-UBT-AppId", strAppId.toUtf8());
    request.setRawHeader("X-UBT-DeviceId", strDeviceId.toUtf8());
    request.setRawHeader("X-UBT-language", strLanguage.toUtf8());
    request.setRawHeader("authorization", Token.toUtf8());


    request.setAttribute(QNetworkRequest::Attribute(Constants::HTTPRequestID),qVariantFromValue((int)nNewRequestID));//设置ID
    request.setAttribute(QNetworkRequest::Attribute(Constants::HTTPRequestType), Constants::HTTP_REQUEST_TRANSFER_UCODE_TOKEN);//设置类型
    QNetworkReply* reply = m_pNetManager->get(request);

    connect(reply, &QNetworkReply::finished, this, &HttpTransfer::OnFinished);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &HttpTransfer::OnError);
    connect(reply, &QNetworkReply::readyRead, this, &HttpTransfer::readyRead);

    m_mapReplyFileLst.insert(nNewRequestID, reply);
}

/************************************
* 名称: OnFinished
* 功能: 指令请求完成槽函数
* 参数: [in]QNetworkReply* reply请求对象指针
* 返回:   void
* 时间:   2020/03/24
* 作者:   Joker
************************************/
void HttpTransfer::OnFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(reply)
    {
        int nRequestID = reply->request().attribute(QNetworkRequest::Attribute(Constants::HTTPRequestID)).toInt();
        //        qint64 nActionID = reply->request().attribute(QNetworkRequest::Attribute(Constants::HTTPFileID)).toLongLong();
        const QString sRequestType = reply->request().attribute(QNetworkRequest::Attribute(Constants::HTTPRequestType)).toByteArray();

        //上传动作的应答
        //        emit sigUploadActionResponse(sRequestType, nRequestID, nActionID, m_bytesResponse);

        //        if (sRequestType == Constants::HTTP_ADD_ACTION_COMMENT)
        //        {
        //            AlphaRobotLog::getInstance()->printDebug(QString("[Httptransfer OnFinished] RequestType: %1, Http response: %2").arg(sRequestType).arg(QString(m_bytesResponse)));
        //            emit sigOnHttpRequestFinished(nRequestID, m_bytesResponse);
        //        }
       UBTUtil::print_msg("HttpTransfer::OnFinished sRequestType = %s\n", sRequestType.toUtf8().data());

       if (sRequestType == Constants::HTTP_REQUEST_GET_FILE_LIST) {
           file_list_info_t userdata;
           //解析数据
           QJsonParseError jsonError;
           QJsonDocument parseDocument = QJsonDocument::fromJson(m_strFileData.toUtf8(), &jsonError);
          UBTUtil::print_msg("HttpTransfer::OnFinished m_strFileData = %s \n", m_strFileData.toUtf8().data());
          m_strFileData = "";
           if(QJsonParseError::NoError == jsonError.error)
           {
               if(parseDocument.isObject())
               {
                   QJsonObject obj = parseDocument.object();
                   if(!obj.empty())
                   {
                       //处理应答的消息

                       //先判断code
                       QString strError = "";
                       strError =  obj.take(OSS_KEY_ERROR).toString();
                       if (!strError.isEmpty())
                       {
                           return;
                       }

                       int nCode = -1;
                       nCode =  obj.take(OSS_KEY_CODE).toInt();
                       UBTUtil::print_msg("HttpTransfer::readyRead nCode =   %d\n", nCode);
                       if (nCode != 0)
                       {
                           return;
                       }

                       QJsonArray arrayData =  obj.take(OSS_KEY_DATA).toArray();

                       for (int i = 0; i < arrayData.size(); i++)
                       {
                           QJsonObject objData = arrayData.at(i).toObject();
                           long long nId = 0;
                           nId =  (long long)objData.take(FILE_SYNCHRONIZ_KEY_ID).toDouble();
                           if (nId == 0)
                           {
                               //有可能获取到假数据，这里做一些校验
                               UBTUtil::print_msg("HttpTransfer::readyRead get null id  %lld\n", nId);
                               continue;
                           }
                           int top_id =  objData.take(FILE_SYNCHRONIZ_KEY_TOP_ID).toInt();
                           int fid =  objData.take(FILE_SYNCHRONIZ_KEY_FID).toInt();
                           int type = objData.take(FILE_SYNCHRONIZ_KEY_TYPE).toInt();
                           QString strName = objData.take(FILE_SYNCHRONIZ_KEY_NAME).toString();
                           long int   fileSize = (long int)objData.take(FILE_SYNCHRONIZ_KEY_FILE_SIZE).toDouble();
                           int updateTime  = objData.take(FILE_SYNCHRONIZ_KEY_UPDATE_TIME).toInt();
                           QString strPath= objData.take(FILE_SYNCHRONIZ_KEY_PATH).toString();
                           QString strUrl = objData.take(FILE_SYNCHRONIZ_KEY_URL).toString();
                           QString strOss = objData.take(FILE_SYNCHRONIZ_KEY_OSS_TYPE).toString();
                           int nState = objData.take(FILE_SYNCHRONIZ_KEY_STATE).toInt();
                           if (nState != eFileStateDeleted)
                           {
                               FileListResourceInfo  fileDB ;
                               fileDB.id = nId;
                               fileDB.top_id = top_id;
                               fileDB.fid = fid; //默认为顶级目录，后台传回来，这里先赋值为0
                               fileDB.type = type;
                               fileDB.file_size = fileSize;
                               fileDB.update_time = updateTime;
                               fileDB.state = nState;
                               fileDB.name = strName;
                               fileDB.path = strPath;
                               fileDB.url = strUrl;
                               fileDB.oss_type = strOss;
                               qDebug()<<"FileListDB::selectItems strId = " << nId << "type"<< type << "strName"<< strName<< "fileSize"<< fileSize<< "updateTime"<< updateTime<< "nState"<< nState;
                               userdata.push_back(fileDB);
                           }
                           else
                           {
                               qDebug()<<"FileListDB::selectItems get deleted id  = " << nId;
                           }
                       }
                   }
               }
           }

           int nSyncType= reply->request().attribute(QNetworkRequest::Attribute(Constants::HTTPFlieSyncType)).toInt();
           if (m_pFileSyn) {
               m_pFileSyn->GetFileListResult(userdata, nSyncType);
           }
       }


        if (m_mapReplyFileLst.contains(nRequestID))
        {
            m_mapReplyFileLst.remove(nRequestID);
        }

        reply->deleteLater();
    }
}

/************************************
* 名称: OnError
* 功能: 错误处理槽函数
* 参数: [in]QNetworkReply::NetworkError code请求错误代码
* 返回:   void
* 时间:   2020/03/24
* 作者:   Joker
************************************/
void HttpTransfer::OnError(QNetworkReply::NetworkError code)
{
    UBTUtil::print_msg("HttpTransfer::OnError code = %d\n", code);
    if(code == QNetworkReply::ConnectionRefusedError ||
            code == QNetworkReply::HostNotFoundError ||
            code == QNetworkReply::TimeoutError ||
            code == QNetworkReply::NetworkSessionFailedError)
    {
        //emit serverError();错误处理
    }
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(reply)
    {
        UBTUtil::print_msg("HttpTransfer::OnError readall = %s\n", reply->readAll().toStdString().c_str());
        qDebug()<<"HttpTransfer::OnError readall" <<reply->readAll();
        int nCode = reply->request().attribute( QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug()<<"HttpTransfer::OnError"<<nCode;
        QString requestType = reply->request().attribute(QNetworkRequest::Attribute(Constants::HTTPRequestType)).toString();
        int nRequestID = reply->request().attribute(QNetworkRequest::Attribute(Constants::HTTPRequestID)).toInt();
        if(requestType == Constants::HTTP_REQUEST_GET_FILE_LIST)
        {
           m_strFileData = "";
        }

        reply->deleteLater();
    }
}

/************************************
* 名称: OnDownloadProgress
* 功能: 下载进度处理槽函数
* 参数: [in]bytesReceived 已经接收到的字节数
* 参数: [in]bytesTotal 总共字节数
* 返回:   void
* 时间:   2020/03/24
* 作者:   Joker
************************************/
void HttpTransfer::OnDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(reply && bytesReceived != 0)
    {
        // 注意：这里会触发两次信号，
        // 第一次：bytesReceived和bytesTotal不相等
        // 第二次：bytesReceived和bytesTotal相等
        // 必须要等待接收数据和总数据相等的时候再读取数据
        // 否则第二次读取到的数据是0
        if (bytesReceived != bytesTotal)
        {
            return;
        }

        QByteArray data = reply->readAll();
        if (data.size() <= 0)
        {
            return;
        }

        QString requestType = reply->request().attribute(QNetworkRequest::Attribute(Constants::HTTPRequestType)).toString();
        int nHTTPRequestID = reply->request().attribute(QNetworkRequest::Attribute(Constants::HTTPRequestID)).toInt();
        UBTUtil::print_msg("HttpTransfer::OnDownloadProgress requestType = %s\n", requestType.toUtf8().data());
        //ActionList 请求
//        if(requestType == Constants::HTTP_REQUEST_GET_FILE_LIST)
//        {
//            m_strFileData
//        }

    }
}


/************************************
* 名称: OnInit
* 功能: 初始化槽函数
* 返回:   void
* 时间:   2020/04/07
* 作者:   Joker
************************************/
void HttpTransfer::OnInit()
{
    m_pNetManager = new QNetworkAccessManager();
}

/**************************************************************************
* 函数名: readyRead
* 功能: 读取后台返回结果，因获取文件列表数据很大，会分批返回，故获取文件列表以OnFinished为准
* 参数: [@in]
* 参数: [@in]
* 返回值:void
* 时间:2020/06/22
* 作者: Joker
*/
void HttpTransfer::readyRead()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(reply)
    {
        int nRequestID = reply->request().attribute(QNetworkRequest::Attribute(Constants::HTTPRequestID)).toInt();
        const QString sRequestType = reply->request().attribute(QNetworkRequest::Attribute(Constants::HTTPRequestType)).toByteArray();
        QByteArray arrData = reply->readAll();
        UBTUtil::print_msg("HttpTransfer::readyRead sRequestType  =  %s\n", sRequestType.toUtf8().data());
        UBTUtil::print_msg("HttpTransfer::readyRead arrData  =  %s\n", arrData.data());
        //        //上传动作的应答
        //        emit sigUploadActionResponse(sRequestType, nRequestID, nActionID, reply->readAll());
        if(sRequestType == Constants::HTTP_REQUEST_ALY_KEY) {
            QJsonParseError jsonError;
            QJsonDocument parseDocument = QJsonDocument::fromJson(arrData, &jsonError);
            int nRet = -1;

            if(QJsonParseError::NoError == jsonError.error)
            {
                if(parseDocument.isObject())
                {
                    QJsonObject obj = parseDocument.object();
                    if(!obj.empty())
                    {
                        //处理应答的消息
                        QString strAccessKeyId =  obj.take(OSS_KEY_NAME_ACCESSKEYID).toString();
                        QString strAccessKeySecret =  obj.take(OSS_KEY_NAME_ACCESSKEYSECRET).toString();
                        QString strEndpoint =  obj.take(OSS_KEY_NAME_ENDPOINT).toString();
                        QString strBucketName =  obj.take(OSS_KEY_NAME_BUCKETNAME).toString();
                        QString strSecurityToken =  obj.take(OSS_KEY_NAME_SECURITYTOKEN).toString();

                        QString strObjectName=  obj.take(OSS_KEY_NAME_OBJECT_NAME).toString();

                        QJsonValue value = obj.take(OSS_KEY_NAME_EXIRETIMESTAMP);

                        long long nExpireTimestamp = (long long)value.toDouble();
                        QString strExpireTimestamp =  QString::number(nExpireTimestamp);
                        CIniConfig::getInstance().setValueAccessKeyId(strAccessKeyId);
                        CIniConfig::getInstance().setValueAccessKeySecret(strAccessKeySecret);
                        CIniConfig::getInstance().setValueEndpoint(strEndpoint);
                        CIniConfig::getInstance().setValueBucketName(strBucketName);
                        CIniConfig::getInstance().setValueObjectName(strObjectName);
                        CIniConfig::getInstance().setValueSecurityToken(strSecurityToken);
                        CIniConfig::getInstance().setValueExpireTimestamp(strExpireTimestamp);
                    }
                }
            }
        }
        else if (sRequestType == Constants::HTTP_REQUEST_GET_FILE_LIST) {
            m_strFileData += arrData;
            //            emit SigGetFileListResult();

        } else if(sRequestType == Constants::HTTP_REQUEST_TRANSFER_UCODE_TOKEN) {
            QJsonParseError jsonError;
            QJsonDocument parseDocument = QJsonDocument::fromJson(arrData, &jsonError);
            QString uCodeToken;

            if(QJsonParseError::NoError == jsonError.error) {
                QJsonObject data = parseDocument.object();
                uCodeToken = data.take("data").toString();
            }

            if (m_pFileSyn) {
                m_pFileSyn->OnGetUCodeToken(uCodeToken);
            }
        }
        else if (sRequestType == Constants::HTTP_REQUEST_GET_OTA_VERSION)
        {
            QJsonParseError jsonError;
            QJsonDocument parseDocument = QJsonDocument::fromJson(arrData, & jsonError);
            int nRet = -1;
            bool bReply = reply->request().attribute(QNetworkRequest::Attribute(Constants::HTTPOtaNeedReply)).toBool();
            if(QJsonParseError::NoError == jsonError.error)
            {
                if(parseDocument.isArray())
                {
                    QJsonArray arrayData =  parseDocument.array();
                    OTAVersionInfo versionInfo;
                    bool bHasNewVersion = true;
                    if (arrayData.size() <= 0)
                    {
                        bHasNewVersion = false;
                    }
                    for (int i = 0; i < arrayData.size(); i++)
                    {
                        QJsonObject objData = arrayData.at(i).toObject();
                        QString strModuleName = objData.take(OTA_KEY_MODULE_NAME).toString();
                        QString strVersionName = objData.take(OTA_KEY_VERSION_NAME).toString();
                        QString strUrl = objData.take(OTA_KEY_PACKAGE_URL).toString();
                        QString strPackegeMd5 = objData.take(OTA_KEY_PACKAGE_MD5).toString();
                        long long nFileSize = (long long)objData.take(OTA_KEY_PACKAGE_SIZE).toDouble();
                        bool bIsForced = objData.take(OTA_KEY_ISFORCED).toBool(false);
                        QString strNote = objData.take(OTA_KEY_RELEASE_NOTE).toString();
                        versionInfo.packageSize = nFileSize;
                        versionInfo.isForced = bIsForced;
                        versionInfo.moduleName = strModuleName;
                        versionInfo.versionName = strVersionName;
                        versionInfo.packageUrl = strUrl;
                        versionInfo.packageMd5 = strPackegeMd5;
                        versionInfo.releaseNote = strNote;
                        UBTUtil::print_msg("QTA get info strModuleName = %s, strVersionName = %s\n", strModuleName.toUtf8().data(), strVersionName.toUtf8().data());
                        UBTUtil::print_msg("QTA get info packageUrl = %s, packageMd5 = %s\n", strUrl.toUtf8().data(), strPackegeMd5.toUtf8().data());
                        UBTUtil::print_msg("QTA get info packageSize = %lld, isForced = %d,  releaseNote= %s\n", nFileSize, bIsForced, strNote.toUtf8().data());

                    }
                    if (m_pFileSyn) {
                        m_pFileSyn->OnGetOtaNewVersion(bReply, bHasNewVersion, versionInfo);
                    }
                }
//                [{
//                    "moduleName": "web",
//                    "versionName": "v1.0.0.2",
//                    "isIncremental": false,
//                    "packageUrl": "https://testqiniu.ubtrobot.com/upgrade/2020/06/1591356139051/ubt-teacher.rar",
//                    "packageMd5": "99336947860306fdbf96f0eca3aabb60",
//                    "packageSize": 26248209,
//                    "isForced": false,
//                    "releaseNote": "d",
//                    "releaseTime": 1591358979
//                }]

            }
        } else if(sRequestType == Constants::HTTP_REQUEST_PUSH_FILE) {
            QJsonParseError jsonError;
            QJsonDocument parseDocument = QJsonDocument::fromJson(arrData, &jsonError);
            int nRet = -1;

            QJsonObject requestBody = QJsonDocument::fromJson(m_mapParamData[nRequestID]).object();
            if(QJsonParseError::NoError == jsonError.error) {
                 int code = parseDocument.object().take("code").toInt();
                if(code == 0 && this->m_pFileSyn) {
                    this->m_pFileSyn->dbDeleteFile(eLoginOnline, requestBody.take("tmpId").toString().toLongLong());
                    this->m_pFileSyn->uploadUCodeFileFinished(true);
                } else {
                    this->m_pFileSyn->uploadUCodeFileFinished(false, code);
                    this->m_pFileSyn->dbDeleteFile(eLoginOnline, requestBody.take("tmpId").toString().toLongLong());
                }
            } else {
                UBTUtil::print_msg("reply error:%d\n",(int)reply->error());
                this->m_pFileSyn->uploadUCodeFileFinished(false, (int)reply->error());
                this->m_pFileSyn->dbDeleteFile(eLoginOnline, requestBody.take("tmpId").toString().toLongLong());
            }
        }

        //        qDebug()<< "HttpTransfer::readyRead data = "<<arrData;
        ////        m_bytesResponse.append(arrData);
        //        qDebug()<< "HttpTransfer::readyRead : m_bytesResponse" << m_bytesResponse;
    }
}
