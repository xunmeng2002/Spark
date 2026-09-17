#pragma once
#include <Spark/Network/NetworkExport.h>
#include "Tcp/SocketInit.h"
#include <string>

namespace Spark::Network
{
class TcpConnect;
class NETWORK_EXPORTS SocketNotify
{
public:
    SocketNotify();
    ~SocketNotify();

    bool Init();
    bool Notify();
    bool Consume();
    SOCKET GetReadSocket();
    TcpConnect* GetConnect();

private:
    bool CreateSocketPair();

    SOCKET sockets_[2];
    TcpConnect* tcpConnect_;

    std::string ip_;
    addrinfo* addressInfo_;
    char receiveBuffer_[16];
};
}

