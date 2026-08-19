#pragma once
#include "Tcp/TcpEpoll/TcpEpollBase.h"


namespace spark::network
{
class TcpEpollServer : public TcpEpollBase
{
public:
	TcpEpollServer(const char* addressName, int milliSeconds);
	~TcpEpollServer();

	virtual bool Init() override;

};
}
