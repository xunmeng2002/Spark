#pragma once
#include "Tcp/TcpBase.h"
#include "Tcp/TcpConnect.h"
#include <list>
#include <map>
#ifdef __linux__
#include <sys/epoll.h>
#endif


namespace spark::network
{
#define EpollEventNumber 1024

class TcpEpollBase : public TcpBase
{
public:
	TcpEpollBase(ServerTypeType serverType, const char* addressName, int milliSeconds);
	~TcpEpollBase();
	
	virtual bool Init() override;
protected:
	virtual void HandleTcpEvent() override;

	virtual void AddConnect(Connect* connect) override;
	virtual void RemoveConnect(Connect* connect) override;
	

	void AddEpollEvent(TcpConnect* connect);
	void RemoveEpollEvent(TcpConnect* connect);
	void AddWriteEpollEvent(TcpConnect* connect);
	void RemoveWriteEpollEvent(TcpConnect* connect);
	
protected:
	int m_EpollFd;
#ifdef __linux__
	epoll_event m_EpollEvents[EpollEventNumber];
#endif
};
}

