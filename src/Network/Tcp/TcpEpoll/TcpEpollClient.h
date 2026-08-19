#pragma once
#include "Tcp/TcpEpoll/TcpEpollBase.h"


namespace spark::network
{
class TcpEpollClient : public TcpEpollBase
{
public:
	TcpEpollClient(const char* addressName, int milliSeconds);
	~TcpEpollClient();

	virtual bool ConnectToServer(const char* ip, unsigned short port) override;

protected:
	void AddEpollConnectEvent(TcpConnect* connect);
};
}
