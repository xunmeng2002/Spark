#pragma once
#ifdef WINDOWS
#include "Tcp/TcpIocp/TcpIocpBase.h"


namespace spark::network
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
	addrinfo* m_ClientLocalAddressInfo;
};
}
#endif // WINDOWS

