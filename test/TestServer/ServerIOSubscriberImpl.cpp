#include "ServerIOSubscriberImpl.h"
#include <Spark/Core/Logger/Logger.h>
#include <Spark/Core/Platform/Platform.h>
#include <string.h>
#include <iostream>

using namespace Spark::Core;
using namespace Spark::Network;

ServerIOSubscriberImpl::ServerIOSubscriberImpl(IOBase* io, IOThread* ioThread)
    :io_(io), ioThread_(ioThread)
{
    io_->Subscribe(this);
}
ServerIOSubscriberImpl::~ServerIOSubscriberImpl()
{
    io_->UnSubscribe();
}

void ServerIOSubscriberImpl::OnConnect(SessionIdType sessionId, const char* ip, int port)
{
    WriteLog(LogLevel::Info, "ServerIOSubscriberImpl::OnConnect SessionId:[%lld], IP:[%s], Port:[%d]", sessionId, ip, port);
}
void ServerIOSubscriberImpl::OnDisConnect(SessionIdType sessionId, const char* ip, int port)
{
    WriteLog(LogLevel::Info, "ServerIOSubscriberImpl::OnDisConnect SessionId:[%lld], IP:[%s], Port:[%d]", sessionId, ip, port);
}



void ServerIOSubscriberImpl::OnRecv(SessionIdType sessionId, Buffer<BuffSize>* buffer)
{
    auto count = ++m_MessageCounts[sessionId];
    //if (count % 1000 == 0)
    {
        char message[2048] = { 0 };
        auto n = sprintf(message, "ServerIOSubscriberImpl::OnRecv SessionId:[%lld], Data:[%s]", (long long)sessionId, buffer->GetData());
        WriteLog(LogLevel::Info, message);
    }

    auto responseBuffer = new Buffer<BuffSize>();
    responseBuffer->Append(buffer->GetData(), buffer->GetLength());
    io_->Send(sessionId, responseBuffer);
}
