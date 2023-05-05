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
#ifndef HTTPTRANSFER_H
#define HTTPTRANSFER_H
//#include "UBXPublic.h"
//#include "ActUpdateMangr.h"
#include <QObject>
#include <QMutexLocker>
#include <QMutex>
#include <QThread>
#include <QMap>
#include <QVector>

class HttpTransfer : public QObject
{
    Q_OBJECT
public:
    HttpTransfer(ILibFileSynchronize* pFileSyn);
    ~HttpTransfer();

public:
    /************************************
    * 名称: stopThread
    * 功能: 退出线程
    * 参数:
    * 返回:
    * 时间:   2020/04/06
    * 作者:   Joker
    ************************************/
    void stopThread();

public:
    /************************************
    * 名称: postQueryActList
    * 功能: post http请求，内部封装了多线程处理
    * 参数: [in]serverURL 请求资源地，
    * 参数: [in]data 请求参数数据
    * 参数: [in]nOldRequestID 上次请求的ID，若还没完成，可以结束上次请求
    * 返回:   int 返回请求ID,外部监听信号时用于判断是否是自己需要的消息
    * 时间:   2020/03/24
    * 作者:   Joker
    ************************************/
    int postQueryFileList(const QString& serverURL, const QString& data, int nOldRequestID);

    /************************************
    * 名称: getQueryFileList
    * 功能: post http请求，内部封装了多线程处理
    * 参数: [in]serverURL 请求资源地，
    * 参数: [in]data 请求参数数据
    * 参数: [in]nOldRequestID 上次请求的ID，若还没完成，可以结束上次请求
    * 返回:   int 返回请求ID,外部监听信号时用于判断是否是自己需要的消息
    * 时间:   2020/03/24
    * 作者:   Joker
    ************************************/
    int getQueryFileList(const QString& serverURL, int nOldRequestID, int nSyncType);


    /************************************
    * 名称: getQueryAlyOssKey
    * 功能: post http请求，内部封装了多线程处理
    * 参数: [in]serverURL 请求资源地，
    * 参数: [in]data 请求参数数据
    * 参数: [in]nOldRequestID 上次请求的ID，若还没完成，可以结束上次请求
    * 返回:   int 返回请求ID,外部监听信号时用于判断是否是自己需要的消息
    * 时间:   2020/03/24
    * 作者:   Joker
    ************************************/
    int getQueryAlyOssKey(const QString& serverURL, int nOldRequestID);


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
    int getQueryOTAVersion(const QString& serverURL, int nOldRequestID, bool bReply);

    QString getUCodeToken(const QString& serverURL, int nOldRequestID);

signals:
    /************************************
    * 名称: SigPostQueryFileList
    * 功能: 以信号的方式通知http线程进行处理
    * 参数: [in]nNewRequestID 新建的请求ID
    * 参数: [in]serverURL 请求资源地
    * 参数: [in]data 请求参数数据
    * 返回:   void
    * 时间:   2020/03/24
    * 作者:   Joker
    ************************************/
    void SigPostQueryFileList(int nNewRequestID, const QString& serverURL, const QString& data);


    /************************************
    * 名称: SigGetQueryFileList
    * 功能: 以信号的方式通知http线程进行处理
    * 参数: [in]nNewRequestID 新建的请求ID
    * 参数: [in]serverURL 请求资源地
    * 参数: [in]data 请求参数数据
    * 返回:   void
    * 时间:   2020/03/24
    * 作者:   Joker
    ************************************/
    void SigGetQueryFileList(int nNewRequestID, const QString& serverURL, int nSyncType);


    /**************************************************************************
    * 函数名: SigGetFileListResult
    * 功能: 获取云端作品列表
    * 参数: [@in] data 云端数据
    * 返回值:void
    * 时间:2020/06/22
    * 作者: Joker
    */
    void SigGetFileListResult(const file_list_info_t& data);

    /************************************
    * 名称: SigGetQueryAlyOssKey
    * 功能: 以信号的方式通知http线程进行处理
    * 参数: [in]nNewRequestID 新建的请求ID
    * 参数: [in]serverURL 请求资源地
    * 参数: [in]data 请求参数数据
    * 返回:   void
    * 时间:   2020/03/24
    * 作者:   Joker
    ************************************/
    void SigGetQueryAlyOssKey(int nNewRequestID, const QString& serverURL);

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
    void SigGetQueryOTAVersion(int nNewRequestID, const QString& serverURL, bool bReply);

    void SigGetUCodeToken(int nNewRequestID, const QString& serverURL);

    /************************************
    * 名称: SigInit
    * 功能: 初始化,先运行线程，然后通过信号的方式通知线程初始化,确保网络对象属于线程
    * 返回:   void
    * 时间:   2020/04/07
    * 作者:   Joker
    ************************************/
    void SigInit();

protected:
    // 清除应答数据
    void resetResp();

    // 更新、获取请求ID
    int createUnusedRequestID();

protected slots:
    /************************************
    * 名称: OnPostPushFile
    * 功能: post http请求槽函数
    * 参数: [in]nNewRequestID 新建的请求ID
    * 参数: [in]serverURL 请求资源地，
    * 参数: [in]data 请求参数数据
    * 返回:
    * 时间:   2020/03/24
    * 作者:   Joker
    ************************************/
    void OnPostPushFile(int nNewRequestID, const QString& serverURL, const QString& data);

    /************************************
    * 名称: OnGetFileList
    * 功能: get http请求槽函数
    * 参数: [in]nNewRequestID 新建的请求ID
    * 参数: [in]serverURL 请求资源地，
    * 参数: [in]data 请求参数数据
    * 返回:
    * 时间:   2020/03/24
    * 作者:   Joker
    ************************************/
    void OnGetFileList(int nNewRequestID, const QString& serverURL, int nSyncType);

    /************************************
    * 名称: OnGetQueryAlyOssKey
    * 功能: get http请求槽函数
    * 参数: [in]nNewRequestID 新建的请求ID
    * 参数: [in]serverURL 请求资源地，
    * 参数: [in]data 请求参数数据
    * 返回:
    * 时间:   2020/03/24
    * 作者:   Joker
    ************************************/
    void OnGetQueryAlyOssKey(int nNewRequestID, const QString& serverURL);

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
    void OnGetQueryOTAVersion(int nNewRequestID, const QString& serverURL, bool bReply);


    void onTokenToUCodeToken(int nNewRequestID, const QString& serverURL);

    /************************************
    * 名称: OnFinished
    * 功能: 指令请求完成槽函数
    * 返回:   void
    * 时间:   2020/03/24
    * 作者:   Joker
    ************************************/
    void OnFinished();

    /************************************
    * 名称: OnError
    * 功能: 错误处理槽函数
    * 参数: [in]QNetworkReply::NetworkError code请求错误代码
    * 返回:   void
    * 时间:   2020/03/24
    * 作者:   Joker
    ************************************/
    void OnError(QNetworkReply::NetworkError code);

    /************************************
    * 名称: OnDownloadProgress
    * 功能: 下载进度处理槽函数
    * 参数: [in]bytesReceived 已经接收到的字节数
    * 参数: [in]bytesTotal 总共字节数
    * 返回:   void
    * 时间:   2020/03/24
    * 作者:   Joker
    ************************************/
    void OnDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

    void OnInit();

    /**************************************************************************
    * 函数名: readyRead
    * 功能: 读取后台返回结果，因获取文件列表数据很大，会分批返回，故获取文件列表以OnFinished为准
    * 参数: [@in]
    * 参数: [@in]
    * 返回值:void
    * 时间:2020/06/22
    * 作者: Joker
    */
    void readyRead();

private:
    QNetworkAccessManager *m_pNetManager;//必须在线程中new，否则get回来的reply对象会变成在主线程中，跨线程调用会崩溃
    //方式是，先运行线程，然后通过信号的方式通知线程初始化
    QThread m_oThread;//线程对象

    QMutex m_mutex; // 互斥锁
    QAtomicInt  m_nRequestID;//生成请求ID

    //Qt帮助文档对post请求的说明，因此需要保存data数据，以防发生意外，在完成后清空保存的数据
    //data must be open for reading and must remain valid until the finished() signal is emitted for this reply.
    QMap<int, QByteArray> m_mapParamData;//保存请求数据
    QMap<int, QByteArray> m_mapRecivedData;//根据请求ID保存读取到的数据
    QMap<int, QNetworkReply*> m_mapReplyFileLst;//File List请求
    QByteArray   m_bytesResponse;  //应答数据
    std::vector<std::string> getHttpHeader(const std::string &strDeviceId);
    ILibFileSynchronize* m_pFileSyn;
    QString m_strFileData;  //文件列表数据，收到数据进行添加，最后再onFinish中使用并清空
};
#endif
