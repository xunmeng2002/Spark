#pragma once
#include "Tcp/TcpEpoll/TcpEpollBase.h"

namespace Spark::Network
{
class TcpEpollServer : public TcpEpollBase
{
public:
    TcpEpollServer(const char* addressName, int milliSeconds);
    ~TcpEpollServer();

    virtual bool Init() override;
};
}
