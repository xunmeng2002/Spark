#pragma once
#include <Spark/Network/NetworkExport.h>
#include <Spark/Core/Platform/Platform.h>
#include <mutex>
#ifdef _WIN32
#include <Ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib")
#endif // _WIN32
#ifdef __linux__
#include <arpa/inet.h>
#include <netdb.h>
#include<unistd.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>

typedef int SOCKET;
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)

#define closesocket close
#endif // __linux__

namespace spark::network
{
class NETWORK_EXPORTS SocketInit
{
public:
	~SocketInit();
	static SocketInit& GetInstance();
	void Init();
private:
	void WsaStart();
private:
	std::once_flag SocketInitFlag;
	static SocketInit _SOCKET_INIT;
};
}
