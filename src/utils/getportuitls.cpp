#include "getportuitls.h"
#include "logHelper.h"

GetPortUitls::GetPortUitls()
{

}

QVector<uint16_t> GetPortUitls::GetAllTcpConnectionsPort()
{
    QVector<uint16_t> ret;
    ULONG size = 0;
    GetTcpTable(NULL, &size, TRUE);
    std::unique_ptr<char[]> buffer(new char[size]);

    PMIB_TCPTABLE tcpTable = reinterpret_cast<PMIB_TCPTABLE>(buffer.get());
    if (GetTcpTable(tcpTable, &size, FALSE) == NO_ERROR)
        for (size_t i = 0; i < tcpTable->dwNumEntries; i++)
            ret.push_back(ntohs((uint16_t)tcpTable->table[i].dwLocalPort));
    std::sort(std::begin(ret), std::end(ret));
    return ret;
}

QVector<uint16_t> GetPortUitls::GetAllUdpConnectionsPort()
{
    QVector<uint16_t> ret;
    ULONG size = 0;
    GetUdpTable(NULL, &size, TRUE);
    std::unique_ptr<char[]> buffer(new char[size]);

    PMIB_UDPTABLE udpTable = reinterpret_cast<PMIB_UDPTABLE>(buffer.get());
    if (GetUdpTable(udpTable, &size, FALSE) == NO_ERROR)
        for (size_t i = 0; i < udpTable->dwNumEntries; i++)
            ret.push_back(ntohs((uint16_t)udpTable->table[i].dwLocalPort));
    std::sort(std::begin(ret), std::end(ret));
    return ret;
}

QVector<uint16_t> GetPortUitls::FindAvailableTcpPort(uint16_t begin, uint16_t end)
{
    auto vec = GetAllTcpConnectionsPort();
    QVector<uint16_t> availablePortVector;
    for (auto port: vec) {
        LOG(INFO)<<"Used port: " <<port;
    }
    //在端口区间检测端口是否在占用列表中，如果不在则表示当前的端口可用并返回
    for (uint16_t port = begin; port != end; ++port){

        if (!std::binary_search(std::begin(vec), std::end(vec), port))
        {
            static int i = 0;
            availablePortVector.insert(i++,port);
        }
    }
    return availablePortVector;
}

QVector<uint16_t> GetPortUitls::FindAvailableUdpPort(uint16_t begin, uint16_t end)
{
    auto vec = GetAllUdpConnectionsPort();
    QVector<uint16_t> availablePortVector;

    for (uint16_t port = begin; port != end; ++port){

        if (!std::binary_search(std::begin(vec), std::end(vec), port))
        {
            static int i = 0;
            availablePortVector.insert(i++,port);
        }
    }
    return availablePortVector;
}

QVector<uint16_t> GetPortUitls::FindAvailablePort(uint16_t begin, uint16_t end)
{
    auto vecTcp = GetAllTcpConnectionsPort(),
            vecUdp = GetAllUdpConnectionsPort();
    QVector<uint16_t> availablePortVector;

    for (uint16_t port = begin; port != end; ++port){
        if (!std::binary_search(std::begin(vecTcp), std::end(vecTcp), port) &&
                !std::binary_search(std::begin(vecUdp), std::end(vecUdp), port))
        {
            static int i = 0;
            availablePortVector.insert(i++,port);
        }
    }
    return availablePortVector;
}
