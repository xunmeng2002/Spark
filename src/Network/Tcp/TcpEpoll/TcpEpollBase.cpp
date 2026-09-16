#include "Tcp/TcpEpoll/TcpEpollBase.h"
#include <Spark/Core/Logger/Logger.h>
#include "Tcp/TcpUtility.h"
#include <string.h>

using namespace Spark::Core;
namespace Spark::Network
{
TcpEpollBase::TcpEpollBase(ServerTypeType serverType, const char* addressName, int milliSeconds)
	:TcpBase(serverType, addressName, milliSeconds), epollFd_(0)
{
#ifdef __linux__
	epollFd_ = epoll_create(5);
#endif
}
TcpEpollBase::~TcpEpollBase()
{
#ifdef __linux__
	close(epollFd_);
#endif
}
bool TcpEpollBase::Init()
{
	if (!TcpBase::Init())
		return false;
	AddEpollEvent(socketNotify_->GetConnect());
	return true;
}
void TcpEpollBase::HandleTcpEvent()
{
#ifdef __linux__
	int number = epoll_wait(epollFd_, epollEvents_, EpollEventNumber, timeOut_.count());
	if (number < 0 && errno != EINTR)
	{
		WriteLog(LogLevel::Info, "epoll wait failed. number:%d, errno:%d\n", number, errno);
		return;
	}
	for (int i = 0; i < number; i++)
	{
		auto epollEvent = epollEvents_[i];
		auto tcpConnect = (TcpConnect*)epollEvents_[i].data.ptr;
		if (serverType_ == ServerTypeType::Server && tcpConnect->SocketId == socket_)
		{
			DoAccept();
		}
		else if (tcpConnect == socketNotify_->GetConnect())
		{
			socketNotify_->Consume();
			for (auto& it : connects_)
			{
				auto connect = it.second;
				if (!connect->Buffers.empty())
				{
					DoSend(connect);
				}
				if (!connect->Buffers.empty())
				{
					AddWriteEpollEvent((TcpConnect*)connect);
				}
			}
		}
		else if (epollEvent.events & EPOLLIN)
		{
			DoRecv(tcpConnect);
		}
		else if (epollEvent.events & EPOLLOUT)
		{
			if (serverType_ == ServerTypeType::Client && connects_.find(tcpConnect->SessionId) == connects_.end())
			{
				int error = 0;
				socklen_t len = sizeof(error);
				int ret = getsockopt(tcpConnect->SocketId, SOL_SOCKET, SO_ERROR, &error, &len);
				if (ret == -1)
				{
					WriteLog(LogLevel::Warning, "getsockopt Failed. SessionId:%lld, SocketId:%lld", tcpConnect->SessionId, tcpConnect->SocketId);
					RemoveConnect(tcpConnect);
					continue;
				}
				if (errno != 0)
				{
					WriteLog(LogLevel::Warning, "Connect Failed. SessionId:%lld, SocketId:%lld, errno:%d", tcpConnect->SessionId, tcpConnect->SocketId, errno);
					RemoveConnect(tcpConnect);
					continue;
				}
				else
				{
					RemoveEpollEvent(tcpConnect);
					AddConnect(tcpConnect);
				}
			}
			else
			{
				DoSend(tcpConnect);
				if (tcpConnect->Buffers.empty())
				{
					RemoveWriteEpollEvent(tcpConnect);
				}
			}
		}
	}
#endif
}
void TcpEpollBase::AddConnect(Connect* connect)
{
	TcpBase::AddConnect(connect);
	AddEpollEvent((TcpConnect*)connect);
}
void TcpEpollBase::RemoveConnect(Connect* connect)
{
	RemoveEpollEvent((TcpConnect*)connect);
	TcpBase::RemoveConnect(connect);
}

void TcpEpollBase::AddEpollEvent(TcpConnect* connect)
{
#ifdef __linux__
	epoll_event epollEvent;
	epollEvent.data.ptr = connect;
	epollEvent.events = EPOLLIN;
	epoll_ctl(epollFd_, EPOLL_CTL_ADD, connect->SocketId, &epollEvent);
#endif
}
void TcpEpollBase::RemoveEpollEvent(TcpConnect* connect)
{
#ifdef __linux__
	epoll_ctl(epollFd_, EPOLL_CTL_DEL, connect->SocketId, NULL);
#endif
}
void TcpEpollBase::AddWriteEpollEvent(TcpConnect* connect)
{
#ifdef __linux__
	epoll_event epollEvent;
	epollEvent.data.ptr = connect;
	epollEvent.events = EPOLLIN | EPOLLOUT;
	epoll_ctl(epollFd_, EPOLL_CTL_MOD, connect->SocketId, &epollEvent);
#endif
}
void TcpEpollBase::RemoveWriteEpollEvent(TcpConnect* connect)
{
#ifdef __linux__
	epoll_event epollEvent;
	epollEvent.data.ptr = connect;
	epollEvent.events = EPOLLIN;
	epoll_ctl(epollFd_, EPOLL_CTL_MOD, connect->SocketId, &epollEvent);
#endif
}
}

