#include "Tcp/TcpIocp/TcpIocpConnect.h"
#include <Spark/TemplateLib/ObjectPool/ObjectPool.h>
#include <Spark/Core/Logger/Logger.h>

#ifdef WINDOWS
using namespace spark::core;

namespace spark::network
{
TcpIocpConnect::TcpIocpConnect(SessionIDType sessionID, const SOCKET& socketID, const std::string& remoteIP, const std::string& remotePort)
	:TcpConnect(sessionID, socketID, remoteIP, remotePort)
{
}
TcpIocpConnect::~TcpIocpConnect()
{
#ifdef WINDOWS
	shutdown(SocketID, SD_BOTH);
#endif
#ifdef LINUX
	shutdown(SocketID, SHUT_RDWR);
#endif
	closesocket(SocketID);
	SocketID = INVALID_SOCKET;
}
TcpIocpConnect* TcpIocpConnect::Allocate(SessionIDType sessionID, const SOCKET& socketID, const std::string& remoteIP, const std::string& remotePort)
{
	return ObjectPool<TcpIocpConnect>::GetInstance().Allocate(sessionID, socketID, remoteIP, remotePort);
}
void TcpIocpConnect::Deallocate()
{
	WriteLog(LogLevel::Info, "TcpIocpConnect::Close SessionID:%lld, Socket:%lld", SessionID, SocketID);
	ObjectPool<TcpIocpConnect>::GetInstance().Deallocate(this);
}

MyOverlapped::MyOverlapped()
{
	Internal = InternalHigh = 0;
	Offset = OffsetHigh = 0;
	hEvent = nullptr;

	EventID = IocpEvent::EventNone;
	MyBuffer = nullptr;
	WsaBuffer.buf = nullptr;
	WsaBuffer.len = 0;
	Connect = nullptr;
}
MyOverlapped::~MyOverlapped()
{
	if (MyBuffer)
	{
		MyBuffer->Reset();
		MyBuffer->Deallocate();
		MyBuffer = nullptr;
	}
	if (Connect)
	{
		Connect = nullptr;
	}
}
MyOverlapped* MyOverlapped::Allocate()
{
	return ObjectPool<MyOverlapped>::GetInstance().Allocate();
}
void MyOverlapped::Deallocate()
{
	ObjectPool<MyOverlapped>::GetInstance().Deallocate(this);
}
void MyOverlapped::SetBuffer(Buffer<BuffSize>* buffer)
{
	if (MyBuffer != nullptr)
	{
		MyBuffer->Deallocate();
	}
	MyBuffer = buffer;
	WsaBuffer.buf = MyBuffer->GetData();
	WsaBuffer.len = MyBuffer->GetLength();
}
void MyOverlapped::Shift(unsigned int len)
{
	MyBuffer->Shift(len);
	WsaBuffer.buf = MyBuffer->GetData();
	WsaBuffer.len = MyBuffer->GetLength();
}
void MyOverlapped::Reset()
{
	Internal = InternalHigh = 0;
	Offset = OffsetHigh = 0;
	hEvent = nullptr;

	EventID = IocpEvent::EventNone;
	if (MyBuffer)
	{
		MyBuffer->Reset();
		WsaBuffer.buf = MyBuffer->GetData();
		WsaBuffer.len = BuffSize;
	}
	else
	{
		WsaBuffer.buf = nullptr;
		WsaBuffer.len = 0;
	}
	Connect = nullptr;
}
}
#endif // WINDOWS
