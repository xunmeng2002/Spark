#pragma once
#ifdef _WIN32
#include "Tcp/TcpIocp/TcpIocpBase.h"


namespace Spark::Network
{
class TcpIocpClient : public TcpIocpBase
{
public:
	TcpIocpClient(const char* addressName, int milliSeconds, int backlog = 5);

	virtual bool Init() override;
	virtual bool ConnectToServer(const char* ip, unsigned short port) override;
protected:
	virtual bool PostConnect() override;
	virtual void OnConnectComplete(MyOverlapped* overlapped) override;

	SOCKET PrepareConnectSocket();

private:
	addrinfo* clientLocalAddressInfo_;
};
}
#endif // _WIN32

