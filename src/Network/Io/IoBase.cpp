#include <Spark/Network/Io/IoBase.h>
#include <Spark/Network/Io/IoUtility.h>
#include <Spark/Core/Utility/TimeUtility.h>
#include <Spark/Core/Logger/Logger.h>

#include "Io/SubscriberNotification.h"

using namespace std;
using namespace Spark::Core;

namespace Spark::Network
{
IoBase::IoBase(ServerTypeType serverType, const char* addressName, int milliSeconds)
    : serverType_(serverType), addressName_(addressName), timeOut_(chrono::milliseconds(milliSeconds)), ioSubscriber_(nullptr), lastSessionIndex_(0LL)
{
    ParseAddress(addressName_, address_, port_);
}
IoBase::~IoBase() {}
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

LinearBuffer<BufferSize>* IoBase::AllocateSendBuffer()
{
    return LinearBuffer<BufferSize>::Allocate();
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
bool IoBase::AddConnect(Connect* connect)
{
    bool wasNewlyRegistered = false;
    {
        std::lock_guard<std::mutex> guard(connectsMutex_);
        wasNewlyRegistered = connects_.insert(std::make_pair(connect->SessionId, connect)).second;
    }
    if (!wasNewlyRegistered)
    {
        const SessionIdType sequenceWithinMillisecond = connect->SessionId % SessionIdSequencePerMillisecond;
        WriteLog(LogLevel::Error,
                 "SessionId Already Registered, Connection Refused. SessionId:%lld, RemoteAddress:%s, RemotePort:%d, Sequence:%zu, Budget:%zu",
                 connect->SessionId, connect->RemoteAddress, connect->RemotePort, static_cast<size_t>(sequenceWithinMillisecond),
                 static_cast<size_t>(SessionIdSequencePerMillisecond));
        return false;
    }
    WriteLog(LogLevel::Info, "New Connection. SessionId:%lld, RemoteAddress:%s, RemotePort:%d", connect->SessionId, connect->RemoteAddress,
             connect->RemotePort);
    if (ioSubscriber_ != nullptr)
    {
        NotifySubscriberSafely("OnConnect", connect->SessionId,
                               [&] { ioSubscriber_->OnConnect(connect->SessionId, connect->RemoteAddress, connect->RemotePort); });
    }
    return true;
}
void IoBase::DiscardRefusedConnect(Connect* connect)
{
    connect->Deallocate();
}
void IoBase::RemoveConnect(Connect* connect)
{
    WriteLog(LogLevel::Info, "RemoveConnect. SessionId:%lld,  RemoteAddress:%s, RemotePort:%d", connect->SessionId, connect->RemoteAddress,
             connect->RemotePort);
    bool wasRegistered = false;
    {
        std::lock_guard<std::mutex> guard(connectsMutex_);
        auto registeredEntry = connects_.find(connect->SessionId);
        wasRegistered = registeredEntry != connects_.end() && registeredEntry->second == connect;
        if (wasRegistered)
        {
            connects_.erase(registeredEntry);
        }
        else if (registeredEntry != connects_.end())
        {
            WriteLog(LogLevel::Error, "SessionId Registered By Another Connect, Entry Kept. SessionId:%lld, RemoteAddress:%s, RemotePort:%d",
                     connect->SessionId, connect->RemoteAddress, connect->RemotePort);
        }
    }
    if (wasRegistered && ioSubscriber_ != nullptr)
    {
        NotifySubscriberSafely("OnDisConnect", connect->SessionId,
                               [&] { ioSubscriber_->OnDisConnect(connect->SessionId, connect->RemoteAddress, connect->RemotePort); });
    }
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
void IoBase::NotifySubscriberRecvSafely(const Connect* connect, const char* data, size_t length)
{
    NotifySubscriberSafely("OnRecv", connect->SessionId, [&] { ioSubscriber_->OnRecv(connect->SessionId, data, length); });
}

SessionIdType IoBase::GetSessionId()
{
    return TimeUtility::GetMilliSecondTimeStamp() * SessionIdSequencePerMillisecond + (++lastSessionIndex_) % SessionIdSequencePerMillisecond;
}
}
