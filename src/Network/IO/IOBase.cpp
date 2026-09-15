#include <Spark/Network/IO/IOBase.h>
#include <Spark/Network/IO/IOUtility.h>
#include <Spark/Core/Utility/TimeUtility.h>
#include <Spark/Core/Logger/Logger.h>

using namespace std;
using namespace Spark::Core;

namespace Spark::Network
{
IOBase::IOBase(ServerTypeType serverType, const char* addressName, int milliSeconds)
	:serverType_(serverType), addressName_(addressName), timeOut_(chrono::milliseconds(milliSeconds)), ioSubscriber_(nullptr), lastSessionIndex_(0LL)
{
	ParseAddress(addressName_, address_, port_);
}
IOBase::~IOBase()
{
}
void IOBase::Subscribe(IOSubscriber* subscriber)
{
	ioSubscriber_ = subscriber;
}
void IOBase::UnSubscribe()
{
	ioSubscriber_ = nullptr;
}
void IOBase::SetTimeOut(int milliSeconds)
{
	timeOut_ = std::chrono::milliseconds(milliSeconds);
}

void IOBase::DisConnect(SessionIDType sessionID)
{
	lock_guard<mutex> guard(disConnectSessionIdsMutex_);
	disConnectSessionIds_.push_back(sessionID);
}
void IOBase::DisConnectAll()
{
	WriteLog(LogLevel::Info, "DisConnectAll");
	std::map<SessionIDType, Connect*> connects(connects_.begin(), connects_.end());
	for (auto& it : connects)
	{
		RemoveConnect(it.second);
	}
}

void IOBase::DoDisConnect()
{
	if (disConnectSessionIds_.empty())
		return;

	lock_guard<mutex> guard(disConnectSessionIdsMutex_);
	for (auto sessionID : disConnectSessionIds_)
	{
		auto connect = connects_[sessionID];
		if (connect == nullptr)
		{
			connects_.erase(sessionID);
		}
		else
		{
			RemoveConnect(connect);
		}
	}
	disConnectSessionIds_.clear();
}
void IOBase::AddConnect(Connect* connect)
{
	WriteLog(LogLevel::Info, "New Connection. SessionID:%lld, RemoteAddress:%s, RemotePort:%d",
		connect->SessionID, connect->RemoteAddress, connect->RemotePort);
	{
		std::lock_guard<std::mutex> guard(connectsMutex_);
		connects_.insert(std::make_pair(connect->SessionID, connect));
	}
	if (ioSubscriber_)
	{
		ioSubscriber_->OnConnect(connect->SessionID, connect->RemoteAddress, connect->RemotePort);
	}
}
void IOBase::RemoveConnect(Connect* connect)
{
	WriteLog(LogLevel::Info, "RemoveConnect. SessionID:%lld,  RemoteAddress:%s, RemotePort:%d",
		connect->SessionID, connect->RemoteAddress, connect->RemotePort);
	if (ioSubscriber_)
	{
		ioSubscriber_->OnDisConnect(connect->SessionID, connect->RemoteAddress, connect->RemotePort);
	}
	std::lock_guard<std::mutex> guard(connectsMutex_);
	connects_.erase(connect->SessionID);
	connect->Deallocate();
}
Connect* IOBase::GetConnect(SessionIDType sessionID)
{
	std::lock_guard<std::mutex> guard(connectsMutex_);
	auto it = connects_.find(sessionID);
	if (it == connects_.end())
	{
		WriteLog(LogLevel::Warning, "Connect not Exist For SessionID:%lld", sessionID);
		return nullptr;
	}
	return it->second;
}


SessionIDType IOBase::GetSessionID()
{
	return TimeUtility::GetMilliSecondTimeStamp() * 100LL + (++lastSessionIndex_) % 100LL;
}
}

