#pragma once
#include "Tcp/TcpSelect/TcpSelectBase.h"


namespace Spark::Network
{
class TcpSelectServer : public TcpSelectBase
{
public:
    TcpSelectServer(const char* addressName, int milliSeconds);
};
}
