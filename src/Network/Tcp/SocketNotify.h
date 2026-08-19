#pragma once
#include <Spark/Network/NetworkExport.h>
#include "Tcp/SocketInit.h"
#include <string>

namespace spark::network
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

private:
	SOCKET m_Sockets[2];
	TcpConnect* m_TcpConnect;

	std::string m_IP;
	addrinfo* m_AddressInfo;
	char m_ReceiveBuffer[16];
};
}

