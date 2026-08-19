#pragma once
#ifdef WINDOWS
#include "Tcp/TcpBase.h"
#include "Tcp/TcpIocp/TcpIocpConnect.h"
#include "Tcp/TcpIocp/TcpIocpCompletePort.h"

namespace spark::network
{
class TcpIocpBase : public TcpBase
{
public:
	TcpIocpBase(ServerTypeType serverType, const char* addressName, int milliSeconds, int backlog = 5);
	~TcpIocpBase();
	
	virtual bool Init() override;
	virtual void Send(SessionIDType sessionID, Buffer<BuffSize>* buffer) override;
protected:
	virtual void HandleTcpEvent() override;
	virtual void DoDisConnect() override;

	virtual bool PostAccept() { return false; }
	virtual bool PostConnect() { return false; }
	virtual bool PostDisConnect(Connect* connect);
	virtual bool PostDisConnect(MyOverlapped* overlapped);
	virtual bool PostSend(MyOverlapped* overlapped);
	virtual bool PostRecv(MyOverlapped* overlapped);

	virtual void OnAcceptComplete(MyOverlapped* overlapped) {}
	virtual void OnConnectComplete(MyOverlapped* overlapped) {}
	virtual void OnDisConnectComplete(MyOverlapped* overlapped);
	virtual void OnSendComplete(MyOverlapped* overlapped, int bytesTransferred);
	virtual void OnRecvComplete(MyOverlapped* overlapped, int bytesTransferred);
protected:
	int m_BackLog;
	IOCompletePort* m_IOCompletePort;
};
}
#endif // WINDOWS

