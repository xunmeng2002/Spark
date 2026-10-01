#include <Spark/Network/Protocol/Protocol.h>
#include <Spark/Core/Logger/Logger.h>
#include <Spark/Network/Io/IoFactory.h>
#include <stdexcept>

#include "Io/SubscriberNotification.h"

using namespace Spark::Core;
namespace Spark::Network
{
static_assert(BufferSize >= MaxFrameSize, "IO 层的收发缓冲必须容纳一帧上限，否则 MakePackage 会写出界");

Protocol::Protocol(ProtocolTypeType protocolType, ServerTypeType serverType, IoModelType ioModel, int milliSeconds,
                   PackageFactoryBase* packageFactory)
    : protocolType_(protocolType), serverType_(serverType), ioModel_(ioModel), milliSeconds_(milliSeconds), ioBase_(nullptr), ioThread_(nullptr),
      packageFactory_(packageFactory), subscriber_(nullptr)
{
}
Protocol::~Protocol()
{
    if (ioBase_ != nullptr)
    {
        delete ioBase_;
        ioBase_ = nullptr;
    }
}
void Protocol::Subscribe(ProtocolSubscriber* subscriber)
{
    subscriber_ = subscriber;
}
void Protocol::UnSubscribe()
{
    subscriber_ = nullptr;
}
void Protocol::RegisterFront(const char* address)
{
    if (ioBase_ != nullptr)
    {
        delete ioBase_;
    }
    ioBase_ = IoFactory::CreateIo(serverType_, address, ioModel_, milliSeconds_);
    ioBase_->Subscribe(this);
    if (ioThread_ != nullptr)
    {
        ioThread_->SetIo(ioBase_);
    }
}
void Protocol::SetIoThread(IoThread* ioThread)
{
    ioThread_ = ioThread;
    if (ioBase_ != nullptr)
    {
        ioThread_->SetIo(ioBase_);
    }
}
void Protocol::SetTimeOut(int milliSeconds)
{
    milliSeconds_ = milliSeconds;
    if (ioBase_ != nullptr)
    {
        ioBase_->SetTimeOut(milliSeconds);
    }
}
bool Protocol::Start()
{
    if (ioThread_ != nullptr)
    {
        return ioThread_->Start();
    }
    return false;
}
void Protocol::Stop()
{
    if (ioThread_ != nullptr)
    {
        ioThread_->Stop();
    }
}
void Protocol::Join()
{
    if (ioThread_ != nullptr)
    {
        ioThread_->Join();
    }
}
bool Protocol::Init()
{
    if (ioBase_ == nullptr)
        return false;
    return ioBase_->Init();
}
IoBase* Protocol::GetIo()
{
    return ioBase_;
}
IoThread* Protocol::GetIoThread()
{
    return ioThread_;
}

void Protocol::DisConnect(SessionIdType sessionId)
{
    if (ioBase_ == nullptr)
        return;
    ioBase_->DisConnect(sessionId);
}
bool Protocol::Send(Package* package)
{
    if (ioBase_ == nullptr)
        return false;
    LinearBuffer<BufferSize>* buffer = LinearBuffer<BufferSize>::Allocate();
    auto len = package->MakePackage(protocolType_, buffer->GetData(), BufferSize);
    if (len <= 0 || !buffer->SetLength(static_cast<size_t>(len)))
    {
        WriteLog(LogLevel::Warning, "MakePackage Failed, Frame Dropped. Len:%d, SessionId:%lld", len, package->SessionId);
        buffer->Deallocate();
        return false;
    }
    ioBase_->Send(package->SessionId, buffer);
    return true;
}

void Protocol::OnConnect(SessionIdType sessionId, const char* ip, int port)
{
    WriteLog(LogLevel::Info, "Protocol::OnConnect SessionId:%lld, IP:%s, Port:%d", sessionId, ip, port);
    PackageReader* packageReader = PackageReader::Allocate(protocolType_, packageFactory_, sessionId, ip);
    if (!sessionPackageReaders_.insert(std::make_pair(sessionId, packageReader)).second)
    {
        WriteLog(LogLevel::Error, "SessionId Already Has PackageReader, Reader Returned. SessionId:%lld", sessionId);
        packageReader->Deallocate();
        return;
    }
    if (subscriber_)
    {
        subscriber_->OnProtocolConnect(sessionId, ip, port);
    }
}
void Protocol::OnDisConnect(SessionIdType sessionId, const char* ip, int port)
{
    WriteLog(LogLevel::Info, "Protocol::OnDisConnect SessionId:%lld, IP:%s, Port:%d", sessionId, ip, port);
    auto it = sessionPackageReaders_.find(sessionId);
    if (it != sessionPackageReaders_.end())
    {
        it->second->Deallocate();
        sessionPackageReaders_.erase(it);
    }
    if (subscriber_)
    {
        subscriber_->OnProtocolDisConnect(sessionId, ip, port);
    }
}
void Protocol::OnRecv(SessionIdType sessionId, const char* data, size_t length)
{
    if (ioBase_ == nullptr)
    {
        return;
    }
    auto it = sessionPackageReaders_.find(sessionId);
    if (it == sessionPackageReaders_.end() || it->second == nullptr)
    {
        WriteLog(LogLevel::Error, "Cannot Find PackageReader for SessionId:%lld", sessionId);
        ioBase_->DisConnect(sessionId);
        return;
    }
    auto packageReader = it->second;
    packageReader->Append(data, length);
    while (true)
    {
        Package* package = nullptr;
        if (!packageReader->ParsePackage(package))
        {
            ioBase_->DisConnect(sessionId);
            break;
        }
        else if (package == nullptr)
        {
            break;
        }
        else if (subscriber_ != nullptr)
        {
            NotifySubscriberSafely("OnMessage", sessionId, [&] { subscriber_->OnMessage(package); });
        }
        else
        {
            package->Deallocate();
        }
    }
}
}
