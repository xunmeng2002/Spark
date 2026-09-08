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
	virtual void AddConnect(Connect* connect) override;
	virtual void RemoveConnect(Connect* connect) override;
protected:
	virtual void DoSend(Connect* connect) override;
	virtual void DoRecv(Connect* connect) override;
	virtual void DoAccept();
	virtual void HandleTcpEvent() = 0;
	virtual void CheckConnect() {}

	// Client 断线自动重连:IO 循环内检测无连接且无在途连接时按固定间隔重试
	void TryAutoReconnect();

protected:
	addrinfo* m_AddressInfo;
	SOCKET m_Socket;
	SocketNotify* m_SocketNotify;

	// 已发起未落定的连接(首连与重连共用):AddConnect/RemoveConnect 落定时复位
	bool m_AutoConnectPending;
	std::chrono::steady_clock::time_point m_LastConnectAttemptTime;


	std::mutex m_ConnectDataMutex;
	
	sockaddr_storage m_RemoteAddress;
#ifdef _WIN32
	int m_RemoteAddressLen;
#endif
#ifdef __linux__
	unsigned int m_RemoteAddressLen;
#endif
};
}

