#pragma once
#ifdef WINDOWS
#include <Spark/Types.h>
#include "Tcp/SocketInit.h"
#include "Tcp/TcpConnect.h"
#include <Spark/TemplateLib/Buffer/Buffer.h>

namespace spark::network
{
enum class IocpEvent : int
{
	EventNone = 0,			//请求连接
	EventConnect = 1,			//请求连接
	EventDisConnect = 2,		//请求断开连接
	EventAccept = 3,			//请求接受连接
	EventOnConnected = 4,		//已连接
	EventOnDisConnected = 5,	//已断开连接
	EventOnAccept = 6,			//已接受连接
	EventSend = 7,				//请求发送
	EventRecv = 8,				//请求接收
};

class TcpIocpConnect : public TcpConnect
{
public:
	TcpIocpConnect(SessionIDType sessionID, const SOCKET& socketID, const std::string& remoteIP, const std::string& remotePort);
	~TcpIocpConnect();
	static TcpIocpConnect* Allocate(SessionIDType sessionID, const SOCKET& socketID, const std::string& remoteIP, const std::string& remotePort);
	virtual void Deallocate() override;

	std::atomic<bool> HasPendingSend = false;
};

class MyOverlapped : public OVERLAPPED
{
public:
	MyOverlapped();
	~MyOverlapped();
	static MyOverlapped* Allocate();
	void Deallocate();
	void SetBuffer(Buffer<BuffSize>* buffer);
	void Shift(unsigned int len);
	void Reset();
	

	IocpEvent EventID;
	WSABUF WsaBuffer;
	Buffer<BuffSize>* MyBuffer;
	TcpIocpConnect* Connect;
};
}
#endif // WINDOWS


