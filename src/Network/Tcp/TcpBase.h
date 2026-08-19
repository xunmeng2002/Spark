#pragma once
#include <Spark/Network/NetworkExport.h>
#include "Tcp/SocketInit.h"
#include "Tcp/TcpConnect.h"
#include "Tcp/SocketNotify.h"
#include <Spark/Network/IO/IOBase.h>
#include <Spark/TemplateLib/Buffer/Buffer.h>
#include <list>
#include <map>
#include <mutex>
#include <string>

namespace spark::network
{
class NETWORK_EXPORTS TcpBase : public IOBase
{
public:
	TcpBase(ServerTypeType serverType, const char* addressName, int milliSeconds);
	virtual ~TcpBase();
	
	virtual bool Init() override;

	virtual void Send(SessionIDType sessionID, Buffer<BuffSize>* buffer) override;
	virtual bool ConnectToServer(const char* address) override;
	virtual bool ConnectToServer(const char* ip, unsigned short port) { return false; }
	virtual void HandleIOEvent() override;
protected:
	virtual void DoSend(Connect* connect) override;
	virtual void DoRecv(Connect* connect) override;
	virtual void DoAccept();
	virtual void HandleTcpEvent() = 0;
	virtual void CheckConnect() {}

protected:
	addrinfo* m_AddressInfo;
	SOCKET m_Socket;
	SocketNotify* m_SocketNotify;


	std::mutex m_ConnectDataMutex;
	
	sockaddr_storage m_RemoteAddress;
#ifdef WINDOWS
	int m_RemoteAddressLen;
#endif
#ifdef LINUX
	unsigned int m_RemoteAddressLen;
#endif
};
}

