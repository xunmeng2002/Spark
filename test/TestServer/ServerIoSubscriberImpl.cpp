#include "ServerIoSubscriberImpl.h"
#include <Spark/Core/Logger/Logger.h>
#include <Spark/Core/Platform/Platform.h>
#include <string.h>
#include <iostream>

using namespace Spark::Core;
using namespace Spark::Network;

ServerIoSubscriberImpl::ServerIoSubscriberImpl(IoBase* io, IoThread* ioThread) : io_(io), ioThread_(ioThread)
{
    io_->Subscribe(this);
}
ServerIoSubscriberImpl::~ServerIoSubscriberImpl()
{
    io_->UnSubscribe();
}

void ServerIoSubscriberImpl::OnConnect(SessionIdType sessionId, const char* ip, int port)
{
    WriteLog(LogLevel::Info, "ServerIoSubscriberImpl::OnConnect SessionId:[%lld], IP:[%s], Port:[%d]", sessionId, ip, port);
}
void ServerIoSubscriberImpl::OnDisConnect(SessionIdType sessionId, const char* ip, int port)
{
    WriteLog(LogLevel::Info, "ServerIoSubscriberImpl::OnDisConnect SessionId:[%lld], IP:[%s], Port:[%d]", sessionId, ip, port);
}

void ServerIoSubscriberImpl::OnRecv(SessionIdType sessionId, Buffer<BufferSize>* buffer)
{
    auto count = ++messageCounts_[sessionId];
    //if (count % 1000 == 0)
    {
        char message[2048] = {0};
        auto n = sprintf(message, "ServerIoSubscriberImpl::OnRecv SessionId:[%lld], Data:[%s]", (long long)sessionId, buffer->GetData());
        WriteLog(LogLevel::Info, message);
    }

    auto responseBuffer = new Buffer<BufferSize>();
    responseBuffer->Append(buffer->GetData(), buffer->GetLength());
    io_->Send(sessionId, responseBuffer);
}
