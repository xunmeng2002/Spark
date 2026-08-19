#pragma once
#include "Tcp/TcpSelect/TcpSelectBase.h"


namespace spark::network
{
class TcpSelectServer : public TcpSelectBase
{
public:
	TcpSelectServer(const char* addressName, int milliSeconds);
};
}
