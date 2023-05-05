#pragma once
#ifndef _WIN32
#ifndef __cplusplus
#define TQLAPCOMM_API extern
#else
#define TQLAPCOMM_API extern "C"
#endif
#else

#include <windows.h>

#ifndef __cplusplus
#ifdef TQLAPCOMM_EXPORTS
#define TQLAPCOMM_API extern	__declspec(dllexport)
#else
#define TQLAPCOMM_API extern	__declspec(dllimport)
#endif
#else
#ifdef TQLAPCOMM_EXPORTS
#define TQLAPCOMM_API extern "C"  __declspec(dllexport)
#else
#define TQLAPCOMM_API extern "C"  __declspec(dllimport)
#endif
#endif
#endif


/** @brief 智能笔坐标点操作类型 */
typedef enum DotType
{
	/** @brief 智能笔按下 */
	PEN_DOWN = 0,

	/** @brief 智能笔移动 */
	PEN_MOVE = 1,

	/** @brief 智能笔弹起 */
	PEN_UP = 2,

	/** @brief 智能笔悬停 */
	PEN_HOVER = 3
}DotType;

/** @brief 智能笔坐标点数据 */
typedef struct Dot
{
	/** @brief 坐标点ID */
	int count;

	/** @brief 智能笔拥有者 */
	int ownerId;

	/** @brief 智能笔按下区域 */
	int sectionId;

	/** @brief 智能笔按下书号 */
	int noteId;

	/** @brief 智能笔按下页号*/
	int pageId;

	/** @brief 智能笔按下横坐标整数部分 */
	int x;

	/** @brief 智能笔按下纵坐标整数部分 */
	int y;

	/** @brief 智能笔按下横坐标小数部分 */
	int fx;

	/** @brief 智能笔按下纵坐标小数部分 */
	int fy;

	/** @brief 智能笔按下压力值 */
	int force;

	/** @brief 坐标点颜色 */
	int color = 0x000000;

	/** @brief 坐标点时间 */
	UINT64 timeLong = 0;

	/** @brief 坐标的角度 */
	int angle;

	/** @brief 坐标点操作类型 */
	DotType type;

	/** @brief 智能笔按下横坐标*/
	float ab_x;

	/** @brief 智能笔按下纵坐标 */
	float ab_y;

	/*unsigned char mac[12];*/
}Dot;


/**
*@brief 获取智能笔坐标点数据回调函数
*@param[in] MACAddr 智能笔的蓝牙MAC地址
*@param[in] dot 智能笔坐标点数据
*@param[in] Count 智能笔的坐标点count
*@param[in] channel 0001 标识在线数据，0002 标识离线数据。
*@return 无
*@see onReceiveDot
**/
typedef void onReceiveDot(char * MACAddr, Dot *dot, int Count,int channel);


/**
*@brief 本地笔迹数据导入点回调
*@param[in] dot    智能笔坐标点数据
*@param[in] Count  智能笔的坐标点count
*@param[in] filter 算法过滤算法
*@return 无
*@see onImportReceiveDot
**/
typedef void onImportReceiveDot(Dot *dot, int Count,bool filter);


/**
*@brief 获取智能笔原始数据(新增)
*@param[in] pBuff 智能笔数据
*@param[in] BuffLength 智能笔数据长度
*@return 无
*@see onReceiveDot
**/
typedef void onDebugger(char * pBuff, int BuffLength);


/**
*@brief 获取TCP数据(新增)
*@param[in] pBuff 智能笔数据
*@param[in] BuffLength 智能笔数据长度
*@return 无
*@see onReceiveDot
**/
typedef void onTcpDataPacket(char * pBuff, int BuffLength);


/**
*@brief 获取智能笔心跳包数据(新增)
*@param[in] Packet 智能笔笔心跳包数据
*@param[in] PacketLength 智能笔笔心跳包数据长度
*@return 无
*@see onHeartbeatPacket
**/
typedef void onHeartbeatPacket(char * Packet, int PacketLength);


/**
*@brief 获取智能笔点读码数据(新增)
*@param[out] SA, SB,SC, SD,Sindex 智能笔笔点读数据
*@return 无
*@see onElementCode
**/
typedef void onElementCode(char * MACAddr, int SA, int SB, int SC, int SD, int Sindex);


/**
*@brief 获取智能笔电量数据(新增)
*@param[in] MACAddr 智能笔的MAC地址
*@param[in] Battery 表示电量值为0-100%
*@param[in] Charging 表示是否充电，0：充电；1：不充电
*@return 无
*@see onPower
**/
typedef void onPower(char * MACAddr, int Battery, int Charging);


/**
*@brief 获取智能笔固件版本数据(新增)
*@param[out] MACAddr 智能笔的MAC地址
*@pBuffer[out] 固件信息
*@Bufferlength[out] 固件信息长度
*@return 无
*@see onFirmwareVersion
**/
typedef void onFirmwareVersion(char * MACAddr, char *FN);


/**
*@brief 获取智能笔名称
*@param[out] MACAddr 智能笔的MAC地址
*@Name[out] 笔名称
*@return 无
*@see onPenName
**/
typedef void onPenName(char * MACAddr, char *Name);


/**
*@brief 获取智能笔离线数据量(新增)
*@param[out] MACAddr 智能笔的MAC地址
*@pBuffer[out] 固件信息
*@Bufferlength[out] 固件信息长度
*@return 无
*@see onFirmwareVersion
**/
typedef void onOffDataNumber(char * MACAddr, unsigned int len);



//输出点原始数据
typedef void onDotOuput(char * MACAddr,char *pBuffer, int Bufferlength);


/*
  获取离线数据量
  Status=0 离线数据结束(关闭) 
  Status=1 开始离线数据 (获取)
  Status=2 离线数据正在上传
*/
typedef void onApOfflineDataStatus(char * MACAddr,int Status);


/*
  客户端状态
  ip 客户端地址
  port 客户端端口
  Status=0 断开连接
  Status=1 已经连接
*/
typedef void onClientStatus(char *ip, int status);


/*
  智能笔 MAC
  mac 智能笔mac 
  当笔有数据传输时候就会触发这个回调，该回调收集所有mac。
*/
typedef void onPenMac(char *mac);



//初始化mac笔
TQLAPCOMM_API void InitCallBackPenMac(onPenMac CallBackPenMac);

//获取笔名回调
TQLAPCOMM_API void InitCallBackPenName(onPenName CallBackPenName);


/**
*@brief 初始化SDK
 Port 默认端口5612
 SocketType 服务类型
*@return 无
*@see Init
**/
TQLAPCOMM_API int Init(int Port, int SocketType);


/**
*@brief 注册获取智能笔坐标点数据回调函数
*@param[in] CallBacReceiveDot 获取智能笔坐标点数据回调函数
*@return 无
*@see InitCallBacReceiveDot
**/
TQLAPCOMM_API void InitCallBackReceiveDot(onReceiveDot CallBacReceiveDot);


/**
*@brief 注册获取智能笔原始数据(新增)
*@param[in] CallBacReceiveDot 获取智能笔原始数据回调函数
*@return 无
*@see InitCallBacReceiveDot
**/
TQLAPCOMM_API void InitCallBackDebugger(onDebugger CallBackDebugger);


/**
*@brief 注册获取智能笔心跳包数据(新增)
*@param[in] onHeartbeatPacket 获取智能笔心跳包数据回调函数
*@return 无
*@see InitCallBackHeartbeatPacket
**/
TQLAPCOMM_API void InitCallBackHeartbeatPacket(onHeartbeatPacket CallBackHeartbeatPacket);


/**
*@brief 注册获取智能笔点读码数据(新增)
*@param[in] onHeartbeatPacket 获取智能笔点读码数据回调函数
*@return 无
*@see InitCallBackElementCode
**/
TQLAPCOMM_API void InitCallBackElementCode(onElementCode CallBackElementCode);

//点原始数据输出
TQLAPCOMM_API void InitCallBackDotOuput(onDotOuput CallBackDotOuput);

//tcp原始数据
TQLAPCOMM_API void  InitCallBackTcpDataPacket(onTcpDataPacket CallBackTcpDatapacket);

//导入离线笔迹数据回调初始化
TQLAPCOMM_API void  InitCallBackImportReceiveDot(onImportReceiveDot CallBackImportReceiveDot);
/**
*@brief 注册获取智能笔电量数据(新增)
*@param[in] onPower 获取智能笔电量数据回调函数
*@return 无
*@see InitCallBackPower
**/
TQLAPCOMM_API void InitCallBackPower(onPower CallBackPower);

//获取离线数据状态
TQLAPCOMM_API void InitCallBackOfflineDataStatus(onApOfflineDataStatus CallBackApOfflineDataNo);

//初始化固件版本回调
TQLAPCOMM_API void InitFirmwareVersion(onFirmwareVersion CallBackFirmwareVersion);

//获取离线数据量
TQLAPCOMM_API void InitOffDataNumber(onOffDataNumber CallBackOffDataNumber);

//客户端状态初始化
TQLAPCOMM_API void InitClientStatus(onClientStatus CallBackClientStatus);
/**
*@brief 结束获取智能笔坐标点数据
*@return 无
*@see CloseReceiveDot
**/
TQLAPCOMM_API void CloseReceiveDot();

////还原点
TQLAPCOMM_API void OpenReceiveDot(unsigned char*lpData, char *mac,bool filter);

TQLAPCOMM_API int  GetApConnectedList(char *ip, char outlist[255][32]);
      
//flag = 0 关闭离线数据
//flag = 1 获取离线数据
//flag = 2 清空离线数据
TQLAPCOMM_API void ApOfflineData(int flag, char *ip, char *mac);

//设置RTC时间   
TQLAPCOMM_API void SetPenRTC(char *ip, char *mac);

//获取笔版本信息
TQLAPCOMM_API void GetPenFirmwareVersion(char *ip, char *mac);

//获取笔数据量大小
TQLAPCOMM_API int GetPenOffDataNumber(char *ip, char *mac);

//获取笔名称
TQLAPCOMM_API void GetPenName(char *ip, char *mac);


////////////////////////////新增无效码设置获取功能////////////////////////////////
/**
*@brief 设置无效码数量返回状态
*@param[out] mac 智能笔mac
*@param[out] statu 状态，statu=0成功，statu=1失败
*@return 无
*@see onCommand
**/
typedef void onCommand(char *mac, int statu);

/**
*@brief 获取智能笔设置无效码
*@param[out] mac  智能笔mac
*@param[out] code 返回的无效码缓存
*@param[out] len  返回的无效码缓存长度
*@return 无
*@see onInvalidCode
**/
typedef void onInvalidCode(char *mac, char *code, int len);


/**
*@brief 获取智能笔设置无效码数量
*@param[out] mac  智能笔mac
*@param[out] opt 返回的数量缓存
*@param[out] len  返回的数量长度
*@return 无
*@see onMcuOptions
**/
typedef void onMcuOptions(char *mac, char *opt, int len);


/**
*@brief 注册设置无效码数量返回状态
*@param[in] CallBackCommand 设置无效码数量返回状态
*@return 无
*@see InitCallBackCommand
**/
TQLAPCOMM_API void InitCallBackCommand(onCommand CallBackCommand);


/**
*@brief 注册获取智能笔设置无效码
*@param[in] CallBackInvalidCode 获取智能笔设置无效码
*@return 无
*@see InitCallBackInvalidCode
**/
TQLAPCOMM_API void InitCallBackInvalidCode(onInvalidCode CallBackInvalidCode);

/**
*@brief 注册获取智能笔设置无效码数量
*@param[in] CallBackMcuOptions 获取智能笔设置无效码数量
*@return 无
*@see InitCallBackMcuOptions
**/
TQLAPCOMM_API void InitCallBackMcuOptions(onMcuOptions CallBackMcuOptions);

/**
*@brief 设置设置无效码数量
*@param[in] ip  AP地址
*@param[in] mac 笔的mac
*@param[in] invalidCount 要设置的数量
*@return 无
*@see SetInvalidCount
**/
TQLAPCOMM_API void SetInvalidCount(char *ip, char *mac, unsigned short invalidCount);

/**
*@brief 设置多项式选项
*@param[in] ip  AP地址
*@param[in] mac 笔的mac
*@param[in] lpOpt 选项 默认是0xFF
*@param[in] Optlen  指示lpOpt的字节长度
*@return 无
*@see SetMcuOptions
**/
TQLAPCOMM_API void SetMcuOptions(char *ip, char *mac, char *lpOpt, unsigned short Optlen);

/**
*@brief 开始断点续传
*@param[in] ip  AP地址
*@param[in] mac 笔的mac
*@return 无
*@see ResumeBreakPoint
**/
TQLAPCOMM_API void startBreakpointResume(char *ip, char *mac);


/**
*@brief 查询是否有断点续传的的点回调
*@param[out] ip  AP地址
*@param[out] mac 笔的mac
*@param[out] has  有true，没有false
*@return 无
*@see onHasHreakpoint
**/
typedef void onHasHreakpoint(char *mac,bool has);


/**
*@brief 注册查询是否有断点续传的的点回调
*@param[in] CallBackInvalidCode 获取智能笔设置无效码
*@return 无
*@see InitCallBackInvalidCode
**/
TQLAPCOMM_API void InitCallBackHasHreakpoint(onHasHreakpoint CallBackHasHreakpoint);

/**
*@brief 查询是否有断点续传的的点
*@param[in] ip  AP地址
*@param[in] mac 笔的mac
*@return 无
*@see ResumeBreakPoint
**/
TQLAPCOMM_API void hasHreakpointContinuationData(char *ip, char *mac);

/**
*@brief 设置书写速度
*@param[in] speed: 0正常书写，1慢速书写，2快速书写,默认是0
*@return 无
*@see setWriteSpeed
**/
TQLAPCOMM_API void setWriteSpeed(char *mac,int speed);



