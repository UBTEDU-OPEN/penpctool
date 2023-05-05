/*
* Copyright (c) 2020, 深圳市优必选科技有限公司
* All rights reserved.
*
* 文件名称：httpdownload.h
* 创建时间：2020/03/31
* 文件标识：
* 文件摘要：下载类
*
* 当前版本：1.0.0.0
* 作    者：Joker
* 完成时间：202020/06/22
* 版本摘要：
*/

#include "httpdownload.h"
#include "techConst.h"
#include "ciniconfig.h"
#include "configs.h"
#include "filedirhandle.h"
#include "ubtutil.h"
//#include "ubxactionlibconstants.h"
//#include "UBXPublic.h"

HttpDownload::HttpDownload(const QString &downloadDir, ILibFileSynchronize *pFileSyn):
    m_pFileSyn(pFileSyn)
{
    m_strDownloadDir = downloadDir;
    m_pNetManager = NULL;
    moveToThread(&m_oThread);
    m_nRequestID = -1;
    connect(this, &HttpDownload::SigToPauseDownload, this, &HttpDownload::OnPauseDownload);
    connect(this, &HttpDownload::SigToResumeDownload, this, &HttpDownload::OnResumeDownload);
    connect(this, &HttpDownload::SigInit, this, &HttpDownload::OnInit);
    connect(this, &HttpDownload::SigDownloadUcd, this, &HttpDownload::OnDownloadUcd);
    connect(this, &HttpDownload::SigDownloadOTAVersion, this, &HttpDownload::OnDownloadOTAVersion);
    m_oThread.start();
    emit SigInit();
}

HttpDownload::~HttpDownload()
{
    stopThread();
    if(m_pNetManager)
    {
        m_pNetManager->disconnect();
        delete m_pNetManager;  // 出现释放打印异常
    }
}

/************************************
* 名称: stopThread
* 功能: 停止线程
* 参数:
* 返回:
* 时间:   2020/04/06
* 作者:   Joker
************************************/
void HttpDownload::stopThread()
{
    m_oThread.quit();
    m_oThread.wait();
}

/************************************
* 名称: DownloadUcd
* 功能: 发送下载文件请求
* 参数: [in]strUrl 下载地址
* 参数: [in]downLoadType 下载类型
* 参数: [in]nChapterId    如果是缓存文件的下载，则需要传章节id
* 参数: [in]nFileId       文件id
* 返回:   int 返回请求ID，外部监听者可以据此判断是否是自己的请求
* 时间:   2020/03/31
* 作者:   Joker
************************************/
int HttpDownload::DownloadUcd(const QString &strUrl, int downLoadType, int nChapterId, int nFileId)
{


    ++m_nRequestID;

    emit SigDownloadUcd(m_nRequestID, strUrl, downLoadType, nChapterId,  nFileId);
    return m_nRequestID;
}

/************************************
* 名称: DownloadOTAVersion
* 功能: 发送下载动作请求
* 参数: [in]strUrl 下载地址
* 参数: [in]downLoadType 下载类型
* 参数: [in]bReply 是否回复web
* 返回:   int 返回请求ID，外部监听者可以据此判断是否是自己的请求
* 时间:   2020/03/31
* 作者:   Joker
************************************/
int HttpDownload::DownloadOTAVersion(const QString &strUrl, int downLoadType, bool bReply)
{
    ++m_nRequestID;

    emit SigDownloadOTAVersion(m_nRequestID, strUrl, downLoadType, bReply);
    return m_nRequestID;
}

/************************************
* 名称: ResumeDownload
* 功能: 重新下载
* 参数: [in]itemData 下载项
* 返回:   int 返回请求ID
* 时间:   2020/04/06
* 作者:   Joker
************************************/
int HttpDownload::ResumeDownload(/*ActItemData itemData*/)
{
    ++m_nRequestID;
//    QFileInfo fileInfo(itemData.m_sActDownloadURL);

//    //在下载目录下先以文件名（不含后缀）创建一个文件夹
//    QDir fileDir;
//    fileDir.setPath(QDir::toNativeSeparators(m_strDownloadDir + "/" + fileInfo.baseName()));
//    if(!fileDir.exists())
//    {
//        fileDir.mkpath(fileDir.absolutePath());
//    }

//    if(!m_oThread.isRunning())
//    {
//        m_oThread.start();
//    }

//    emit SigToResumeDownload(m_nRequestID, itemData);
    return m_nRequestID;
}

/**************************************************************************
* 函数名: GetLocalFileName
* 功能: 根据不同的信息获取本地文件
* 参数: [in]strUrl 下载地址
* 参数: [in]downLoadType 下载类型
* 参数: [in]nChapterId    如果是缓存文件的下载，则需要传章节id
* 参数: [in]nFileId       文件id
* 返回值:void
* 时间:2020/06/22
* 作者: Joker
*/
QString HttpDownload::GetLocalFileName(const QString &strUrl, int nDowmLoadType, int nChapterId, int nFileID)
{


    QString strUserFilePathName = "";
    QString strFileName = "";
    QString strTopDir;

    if (nDowmLoadType == Constants::eHTTPDownloadWebOtaVersion)
    {
        //与用户无关，直接返回路径
        strFileName = CFileDirHandle::getFileName(strUrl);
        strUserFilePathName = CConfigs::getLocalTmpDir() + QDir::separator() + strFileName;
        UBTUtil::print_msg("nDowmLoadType == Constants::eHTTPDownloadWebOtaVersion strUserFilePathName =%s\n", strUserFilePathName.toUtf8().data());
        return strUserFilePathName;
    }

    if (nDowmLoadType == Constants::eHTTPDownloadMainOtaVersion)
    {
        //与用户无关，直接返回路径
        strFileName = CFileDirHandle::getFileName(strUrl);
        strUserFilePathName = CConfigs::getLocalTmpDir() + QDir::separator() + TECH_OTA_MODULE_NAME_MAIN + QDir::separator() + strFileName;
        UBTUtil::print_msg("nDowmLoadType == Constants::eHTTPDownloadMainOtaVersion strUserFilePathName =%s\n", strUserFilePathName.toUtf8().data());
        return strUserFilePathName;
    }

    if (nDowmLoadType == Constants::eHTTPDownloadSubModuleOtaVersion) {
        //与用户无关，直接返回路径
        strFileName = CFileDirHandle::getFileName(strUrl);
        strUserFilePathName = CConfigs::getLocalTmpDir() + QDir::separator() + TECH_OTA_MODULE_NAME_SUB_MODULE + QDir::separator() + strFileName;
        UBTUtil::print_msg("nDowmLoadType == Constants::eHTTPDownloadUCodeOtaVersion strUserFilePathName =%s\n", strUserFilePathName.toUtf8().data());
        return strUserFilePathName;
    }

    if (nDowmLoadType == Constants::eHTTPDownloadfile)
    {
         strTopDir = CConfigs::getLocalFileListDir();
    }
    else
    {
        strTopDir = CConfigs::getLocalCashListFir();
    }

    if ((nDowmLoadType == Constants::eHTTPDownloadCash || nDowmLoadType == Constants::eHTTPDownloadCashAndOpen) && nFileID != 0)
    {
        if (m_pFileSyn)
        {
            strFileName = m_pFileSyn->getFileNamebyId(nChapterId, nFileID,  Constants::eHTTPDownloadCash);
        }
    }
    else if (nDowmLoadType == Constants::eHTTPDownloadfile)
    {
         strFileName = CFileDirHandle::getFileBaseName(strUrl);
         long long nID = strFileName.toLongLong();
         if (m_pFileSyn)
         {
             strFileName = m_pFileSyn->getFileNamebyId(nChapterId, nID,  Constants::eHTTPDownloadfile);
         }
         if (strFileName.isEmpty())
         {
              strFileName = CFileDirHandle::getFileBaseName(strUrl);//如果没有找到文件名，直接沿用旧版本的原文件名
         }
         UBTUtil::print_msg("HttpDownload::HttpDownload::GetLocalFileName strFileName = %s", strFileName.toUtf8().data());

    }
    else
    {
        strFileName = CFileDirHandle::getFileName(strUrl);
//        UBTUtil::print_msg("HttpDownload::HttpDownload::GetLocalFileName else strFileName = %s", strFileName.toUtf8().data());
    }
//            int nIndex = strFileName.indexOf('?');
//            if (nIndex > 0)
//            {
//                strFileName.remove(nIndex, strFileName.length() - nIndex);
//            }
    QString strUserID   = CIniConfig::getInstance().getValueUserID();

    QString strUserDirFile = strTopDir + QDir::separator() + strUserID;

    QString suffix = CFileDirHandle::getFileSuffix(strFileName);

    QString strSuffixPath = "";
    if (nDowmLoadType == Constants::eHTTPDownloadCash || nDowmLoadType == Constants::eHTTPDownloadCashAndOpen)
    {
        if (suffix.compare(UBT_PICTURE_FILE_EXT, Qt::CaseInsensitive) == 0 || suffix.compare(UBT_PICTURE_FILE_EXT_JPG, Qt::CaseInsensitive) == 0)
        {
            strSuffixPath =  UBT_PICTURE_NAME;
        }
        else if (suffix.compare(UBT_VIDEO_FILE_EXT, Qt::CaseInsensitive) == 0)
        {
            strSuffixPath =  UBT_VIDAEO_NAME;
        }
        else
        {
             strSuffixPath =  UBT_FILE_NAME;
        }
    }

    if (!strSuffixPath.isEmpty())
    {
        strUserFilePathName = strUserDirFile + QDir::separator() + strSuffixPath + QDir::separator() + strFileName;
    }
    else
    {
        strUserFilePathName = strUserDirFile + QDir::separator() + strFileName;
    }
//    UBTUtil::print_msg("HttpDownload::HttpDownload::GetLocalFileName strUserFilePathName = %s", strUserFilePathName.toUtf8().data());
    return strUserFilePathName;

}

/************************************
* 名称: OnError
* 功能: 网络错误槽函数
* 参数: [in]code 错误码
* 返回:   void
* 时间:   2020/03/31
* 作者:   Joker
************************************/
void HttpDownload::OnError(QNetworkReply::NetworkError code)
{
    if(code == QNetworkReply::ConnectionRefusedError ||
       code == QNetworkReply::HostNotFoundError ||
       code == QNetworkReply::TimeoutError ||
       code == QNetworkReply::NetworkSessionFailedError)
    {
        //特殊错误处理，暂时没有需求
    }

    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(reply)
    {
        int nFileID = reply->request().attribute(QNetworkRequest::Attribute(Constants::HTTPFileID)).toInt();
        int nChapterID = reply->request().attribute(QNetworkRequest::Attribute(Constants::HTTPChapterID)).toInt();

        int nDowmLoadType = reply->request().attribute(QNetworkRequest::Attribute(Constants::HTTPDownloadType)).toInt();

        if (nFileID != 0)
        {
            if (m_pFileSyn)
            {
                m_pFileSyn->httpDownLoadResult(false, nChapterID, nFileID, nDowmLoadType);
            }
        }

        if (nDowmLoadType == Constants::eHTTPDownloadWebOtaVersion || nDowmLoadType == Constants::eHTTPDownloadMainOtaVersion)
        {
            bool bReply = reply->request().attribute(QNetworkRequest::Attribute(Constants::HTTPOtaNeedReply)).toBool();
            if (m_pFileSyn)
            {
                m_pFileSyn->httpOtaDownLoadResult(false, nDowmLoadType, bReply);
            }
        }
        //下载失败
//        //移除存储的本地路径
//        m_mapIteDada.remove(nActionID);
//        m_mapReply.remove(nActionID);
        reply->deleteLater();
    }
}

/************************************
* 名称: OnDownloadProgress
* 功能: 下载进度处理槽函数
* 参数: [in]bytesReceived 已经接收到的字节数
* 参数: [in]bytesTotal 总共的字节数
* 返回:   void
* 时间:   2020/03/31
* 作者:   Joker
************************************/
void HttpDownload::OnDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if(reply && bytesReceived != 0)
    {
        int nRequestID = reply->request().attribute(QNetworkRequest::Attribute(Constants::HTTPRequestID)).toInt();
        QString strUrl = reply->request().attribute(QNetworkRequest::Attribute(Constants::HTTPDownloadFilePath)).toString();

        int nDowmLoadType = reply->request().attribute(QNetworkRequest::Attribute(Constants::HTTPDownloadType)).toInt();
        int nChapterID = reply->request().attribute(QNetworkRequest::Attribute(Constants::HTTPChapterID)).toInt();
        int nFileID = reply->request().attribute(QNetworkRequest::Attribute(Constants::HTTPFileID)).toInt();
        if (nFileID != 0 || nChapterID != 0)
        {
            UBTUtil::print_msg("HttpDownload::OnDownloadProgress nFileID =%d , nChapterID = %d", nFileID, nChapterID);
        }
//        //通知完成下载
//        emit SigFinishedDownload(nRequestID, nActionID);

        quint64 nbytesAvailable = reply->bytesAvailable();
        if(nbytesAvailable > 0)
        {
            QString strUserFilePathName = GetLocalFileName(strUrl, nDowmLoadType, nChapterID, nFileID);
            qDebug()<< "HttpDownload::OnDownloadProgress strUserFilePathName = "<<strUserFilePathName;
            QFile localFile(strUserFilePathName);

            //不存在则为刚开始下载
            if(!localFile.exists())
            {
//                emit SigStartDownload(nRequestID, m_mapIteDada[nActionID], bytesReceived, bytesTotal);
                if (nDowmLoadType == Constants::eHTTPDownloadMainOtaVersion || nDowmLoadType == Constants::eHTTPDownloadSubModuleOtaVersion) {
                    bool bReply = reply->request().attribute(QNetworkRequest::Attribute(Constants::HTTPOtaNeedReply)).toBool();
                    if (!bReply) {
                        if (m_pFileSyn) {
                            m_pFileSyn->Show_Download_Process(bytesReceived, bytesTotal);
                        }
                    }

                }
                UBTUtil::print_msg("HttpDownload::OnDownloadProgress localFile is not exists strUserFilePathName = %s", strUserFilePathName.toUtf8().data());
            }

            localFile.open(QIODevice::WriteOnly | QIODevice::Unbuffered | QIODevice::Append);
            if (!localFile.isOpen())
            {
                //如果文件无法写入，直接结束本次下载
                UBTUtil::print_msg("HttpDownload::OnDownloadProgress localFile isOpen  abort strUserFilePathName = %s", strUserFilePathName.toUtf8().data());
                reply->abort();
                if (nFileID != 0)
                {
                    if (m_pFileSyn) {
                        m_pFileSyn->httpDownLoadResult(false, nChapterID, nFileID, nDowmLoadType);
                        m_pFileSyn->Hide_Download_Process();
                    }
                }

                if(
                        nDowmLoadType == Constants::eHTTPDownloadWebOtaVersion ||
                        nDowmLoadType == Constants::eHTTPDownloadMainOtaVersion ||
                        nDowmLoadType == Constants::eHTTPDownloadSubModuleOtaVersion
                ) {
                    bool bReply = reply->request().attribute(QNetworkRequest::Attribute(Constants::HTTPOtaNeedReply)).toBool();
                    if (m_pFileSyn)
                    {
                        m_pFileSyn->httpOtaDownLoadResult(false, nDowmLoadType, bReply);
                    }
                }

                return;
            }

            quint64 dataSize = nbytesAvailable;
            quint64 nReadedBytes = 0;
            do
            {
                QByteArray data = reply->read(nbytesAvailable);

                nReadedBytes = dataSize = data.size();
                char *writeBuff = data.data();
                while (dataSize > 0)
                {
                    quint64 writedSize = localFile.write(writeBuff, dataSize);
                    dataSize -= writedSize;
                    writeBuff += writedSize;
                }
                localFile.flush();
                localFile.close();
            }while((nbytesAvailable -= nReadedBytes) > 0);

            if (nDowmLoadType == Constants::eHTTPDownloadMainOtaVersion || nDowmLoadType == Constants::eHTTPDownloadSubModuleOtaVersion) {
                bool bReply = reply->request().attribute(QNetworkRequest::Attribute(Constants::HTTPOtaNeedReply)).toBool();
                if (!bReply) {
                    if (m_pFileSyn) {
                        m_pFileSyn->Show_Download_Process(bytesReceived, bytesTotal);
                    }
                }
            }

            //已经存在则是断点下载，此时的received和total需要自己计算,因此最好不使用槽的参数，而使用自己记录过的值
//            emit SigDownloadProgress(nRequestID, nActionID, m_mapIteDada[nActionID].m_nHasDownloadedBytes, m_mapIteDada[nActionID].m_nActSize);

            //写入本地文件
//            if(bytesReceived == bytesTotal)
//            {
//                //如果是zip文件，则先解压
//                QString strFileSuffix = strFileName.mid(strFileName.lastIndexOf("."));
//                if(strFileSuffix.compare(".zip", Qt::CaseInsensitive) == 0)
//                {
//                    UnZipFile(strFileName);
//                }
//                g_actupdatemgr->AddDownLoadCount(nActionID);
//            }
        }
    }
}

/************************************
* 名称: OnPauseDownload
* 功能: 响应暂停下载槽函数
* 参数: [in]nActionID 指定要暂停的action ID
* 返回:   void
* 时间:   2020/04/06
* 作者:   Joker
************************************/
void HttpDownload::OnPauseDownload(int nActionID)
{
//    if(m_mapReply.contains(nActionID))
//    {
//        m_mapReply[nActionID]->abort();
//        m_mapReply.remove(nActionID);
//        emit SigHasPausedDownload(nActionID);
//    }
}

/************************************
* 名称: OnFinished
* 功能: 指令请求完成槽函数
* 返回:   void
* 时间:   2020/04/06
* 作者:   Joker
************************************/
void HttpDownload::OnFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(reply)
    {
        int nRequestID = reply->request().attribute(QNetworkRequest::Attribute(Constants::HTTPRequestID)).toInt();
        int nChapterID = reply->request().attribute(QNetworkRequest::Attribute(Constants::HTTPChapterID)).toInt();
        int nFileID = reply->request().attribute(QNetworkRequest::Attribute(Constants::HTTPFileID)).toInt();
        int nDowmLoadType = reply->request().attribute(QNetworkRequest::Attribute(Constants::HTTPDownloadType)).toInt();

        quint64 nbytesAvailable = reply->bytesAvailable();
        if(nbytesAvailable > 0)
        {
            UBTUtil::print_msg("HttpDownload::OnFinished nbytesAvailable = %lld", nbytesAvailable);
        }
//        m_mapReply.remove(nActionID);
        reply->deleteLater();

//        //通知完成下载
//        emit SigFinishedDownload(nRequestID, nActionID);
        if (nFileID != 0)
        {
            if (m_pFileSyn)
            {
                m_pFileSyn->httpDownLoadResult(true, nChapterID, nFileID, nDowmLoadType);
            }

        }

        if(
                nDowmLoadType == Constants::eHTTPDownloadWebOtaVersion ||
                nDowmLoadType == Constants::eHTTPDownloadMainOtaVersion ||
                nDowmLoadType == Constants::eHTTPDownloadSubModuleOtaVersion
        ) {
            bool bReply = reply->request().attribute(QNetworkRequest::Attribute(Constants::HTTPOtaNeedReply)).toBool();
            if (m_pFileSyn) {
                m_pFileSyn->httpOtaDownLoadResult(true, nDowmLoadType, bReply);
            }
        }
    }
}

/************************************
* 名称: OnResumeDownload
* 功能: 重新下载
* 参数: [in]nRequestID 请求ID
* 参数: [in]itemData 下载项
* 返回:   void
* 时间:   2020/04/06
* 作者:   Joker
************************************/
void HttpDownload::OnResumeDownload(int nRequestID)
{
//    m_mapIteDada.insert(itemData.m_nItemID, itemData);
//    QNetworkRequest request;
//    request.setUrl(QUrl(itemData.m_sActDownloadURL));

//    QString strRange = "bytes " + QString::number(itemData.m_nHasDownloadedBytes) + "-";
//    request.setRawHeader("Range",strRange.toLatin1());

//    request.setAttribute(QNetworkRequest::Attribute(Constants::HTTPRequestID), (int)nRequestID);
//    request.setAttribute(QNetworkRequest::Attribute(Constants::HTTPFileID), itemData.m_nItemID);
//    QNetworkReply* reply = m_pNetManager->get(request);
//    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
//            this, &HttpDownload::OnError);
//    connect(reply, &QNetworkReply::downloadProgress,
//            this, &HttpDownload::OnDownloadProgress);
//    connect(reply, &QNetworkReply::finished, this, &HttpDownload::OnFinished);
//    m_mapReply.insert(itemData.m_nItemID, reply);
}

/************************************
* 名称: OnDownloadUcd
* 功能: 下载文件
* 参数: [in]strUrl 下载地址
* 参数: [in]downLoadType 下载类型
* 参数: [in]nChapterId    如果是缓存文件的下载，则需要传章节id
* 参数: [in]nFileId       文件id
* 返回:   void
* 时间:   2020/04/06
* 作者:   Joker
************************************/
void HttpDownload::OnDownloadUcd(int nRequestID, const QString &strUrl, int downLoadType,int nChapterId, int nFileId)
{
    QNetworkRequest request;
    request.setUrl(QUrl(strUrl));

    request.setAttribute(QNetworkRequest::Attribute(Constants::HTTPRequestID), (int)nRequestID);
    request.setAttribute(QNetworkRequest::Attribute(Constants::HTTPDownloadFilePath), strUrl);//设置类型
    request.setAttribute(QNetworkRequest::Attribute(Constants::HTTPDownloadType), downLoadType);//设置类型
    request.setAttribute(QNetworkRequest::Attribute(Constants::HTTPFileID), (int)nFileId);//设置类型
    request.setAttribute(QNetworkRequest::Attribute(Constants::HTTPChapterID), (int)nChapterId);//设置类型

    QNetworkReply* reply = m_pNetManager->get(request);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
            this, &HttpDownload::OnError);
    connect(reply, &QNetworkReply::downloadProgress,
            this, &HttpDownload::OnDownloadProgress);
    connect(reply, &QNetworkReply::finished, this, &HttpDownload::OnFinished);
}

/************************************
* 名称: SigDownloadOTAVersion
* 功能: 发送下载动作请求
* 参数: [in]nRequestID 请求id
* 参数: [in]strUrl 下载地址
* 参数: [in]downLoadType 下载类型
* 参数: [in]bReply 是否回复web
* 返回:   int 返回请求ID，外部监听者可以据此判断是否是自己的请求
* 时间:   2020/03/31
* 作者:   Joker
************************************/
void HttpDownload::OnDownloadOTAVersion(int nRequestID, const QString &strUrl, int downLoadType, bool bReply)
{
    QNetworkRequest request;
    request.setUrl(QUrl(strUrl));

    request.setAttribute(QNetworkRequest::Attribute(Constants::HTTPRequestID), (int)nRequestID);
    request.setAttribute(QNetworkRequest::Attribute(Constants::HTTPDownloadFilePath), strUrl);//设置类型
    request.setAttribute(QNetworkRequest::Attribute(Constants::HTTPDownloadType), downLoadType);//设置类型
    request.setAttribute(QNetworkRequest::Attribute(Constants::HTTPOtaNeedReply), bReply);//设置类型


    QNetworkReply* reply = m_pNetManager->get(request);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
            this, &HttpDownload::OnError);
    connect(reply, &QNetworkReply::downloadProgress,
            this, &HttpDownload::OnDownloadProgress);
    connect(reply, &QNetworkReply::finished, this, &HttpDownload::OnFinished);
}

/************************************
* 名称: OnInit
* 功能: 初始化槽函数
* 返回:   void
* 时间:   2020/04/07
* 作者:   Joker
************************************/
void HttpDownload::OnInit()
{
    m_pNetManager = new QNetworkAccessManager();
}


