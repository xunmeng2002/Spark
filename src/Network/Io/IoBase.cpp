#include <Spark/Network/Io/IoBase.h>
#include <Spark/Network/Io/IoUtility.h>
#include <Spark/Core/Utility/TimeUtility.h>
#include <Spark/Core/Logger/Logger.h>

using namespace std;
using namespace Spark::Core;

namespace Spark::Network
{
IoBase::IoBase(ServerTypeType serverType, const char* addressName, int milliSeconds)
    :serverType_(serverType), addressName_(addressName), timeOut_(chrono::milliseconds(milliSeconds)), ioSubscriber_(nullptr), lastSessionIndex_(0LL)
{
    ParseAddress(addressName_, address_, port_);
}
IoBase::~IoBase()
{
}
void IoBase::Subscribe(IoSubscriber* subscriber)
{
    ioSubscriber_ = subscriber;
}
void IoBase::UnSubscribe()
{
    ioSubscriber_ = nullptr;
}
void IoBase::SetTimeOut(int milliSeconds)
{
    timeOut_ = std::chrono::milliseconds(milliSeconds);
}

void IoBase::DisConnect(SessionIdType sessionId)
{
    lock_guard<mutex> guard(disConnectSessionIdsMutex_);
    disConnectSessionIds_.push_back(sessionId);
}
void IoBase::DisConnectAll()
{
    WriteLog(LogLevel::Info, "DisConnectAll");
    std::map<SessionIdType, Connect*> connects(connects_.begin(), connects_.end());
    for (auto& it : connects)
    {
        RemoveConnect(it.second);
    }
}

void IoBase::DoDisConnect()
{
    if (disConnectSessionIds_.empty())
        return;

    lock_guard<mutex> guard(disConnectSessionIdsMutex_);
    for (auto sessionId : disConnectSessionIds_)
    {
        auto connect = connects_[sessionId];
        if (connect == nullptr)
        {
            connects_.erase(sessionId);
        }
        else
        {
            RemoveConnect(connect);
        }
    }
    disConnectSessionIds_.clear();
}
void IoBase::AddConnect(Connect* connect)
{
    WriteLog(LogLevel::Info, "New Connection. SessionId:%lld, RemoteAddress:%s, RemotePort:%d",
        connect->SessionId, connect->RemoteAddress, connect->RemotePort);
    {
        std::lock_guard<std::mutex> guard(connectsMutex_);
        connects_.insert(std::make_pair(connect->SessionId, connect));
    }
    if (ioSubscriber_)
    {
        ioSubscriber_->OnConnect(connect->SessionId, connect->RemoteAddress, connect->RemotePort);
    }
}
void IoBase::RemoveConnect(Connect* connect)
{
    WriteLog(LogLevel::Info, "RemoveConnect. SessionId:%lld,  RemoteAddress:%s, RemotePort:%d",
        connect->SessionId, connect->RemoteAddress, connect->RemotePort);
    if (ioSubscriber_)
    {
        ioSubscriber_->OnDisConnect(connect->SessionId, connect->RemoteAddress, connect->RemotePort);
    }
    std::lock_guard<std::mutex> guard(connectsMutex_);
    connects_.erase(connect->SessionId);
    connect->Deallocate();
}
Connect* IoBase::GetConnect(SessionIdType sessionId)
{
    std::lock_guard<std::mutex> guard(connectsMutex_);
    auto it = connects_.find(sessionId);
    if (it == connects_.end())
    {
        WriteLog(LogLevel::Warning, "Connect not Exist For SessionId:%lld", sessionId);
        return nullptr;
    }
    return it->second;
}


SessionIdType IoBase::GetSessionId()
{
    return TimeUtility::GetMilliSecondTimeStamp() * 100LL + (++lastSessionIndex_) % 100LL;
}
}

