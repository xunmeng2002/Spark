#include "Tcp/SocketInit.h"
#include <iostream>


namespace spark::network
{
SocketInit SocketInit::_SOCKET_INIT;


SocketInit::~SocketInit()
{
#ifdef _WIN32
	WSACleanup();
#endif
}
SocketInit& SocketInit::GetInstance()
{
	return _SOCKET_INIT;
}
void SocketInit::Init()
{
	std::call_once(SocketInitFlag, &SocketInit::WsaStart, this);
}
void SocketInit::WsaStart()
{
#ifdef _WIN32
	std::cout << "ON WINDOWS" << std::endl;
	WSADATA initData;
	initData.wVersion = 0;
	initData.wHighVersion = 2;
	memset(initData.szDescription, 0, sizeof(initData.szDescription));
	memset(initData.szSystemStatus, 0, sizeof(initData.szSystemStatus));
	initData.iMaxSockets = 1024;
	initData.lpVendorInfo = nullptr;
	if (WSAStartup(2, &initData) != 0)
	{
		std::cout << "SocketInit Failed!" << std::endl;
	}
	//WSADATA initData;
	//if (WSAStartup(MAKEWORD(2, 2), &initData))
	//{
	//	std::cout << "SocketInit Failed!" << std::endl;
	//}
#endif
#ifdef __linux__
	std::cout << "ON LINUX" << std::endl;
#endif
}
}
