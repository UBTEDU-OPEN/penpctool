#ifndef GETPORTUITLS_H
#define GETPORTUITLS_H
#include "utilsGlobal.h"
#include <QObject>
#include <Windows.h>
#include <WinSock.h>
#include <tcpmib.h>
#include <iphlpapi.h>
#include <memory>
#include <algorithm>
#include <QVector>
#include <QDebug>

#define PORT_DOWN 5600
#define PORT_UP 65535
class UTILS_EXPORT GetPortUitls
{
public:
    GetPortUitls();
    static QVector<uint16_t> GetAllTcpConnectionsPort();//获取所有已占用的TCP端口号
    static QVector<uint16_t> GetAllUdpConnectionsPort();//获取所有已占用的UDP端口号
    QVector<uint16_t> FindAvailableTcpPort(uint16_t begin = PORT_DOWN, uint16_t end = PORT_UP);//查找可用的TPC端口
    QVector<uint16_t> FindAvailableUdpPort(uint16_t begin = PORT_DOWN, uint16_t end = PORT_UP);//查找可用的UDP端口
    QVector<uint16_t> FindAvailablePort(uint16_t begin = PORT_DOWN, uint16_t end = PORT_UP);//查找可用的端口
};

#endif // GETPORTUITLS_H
