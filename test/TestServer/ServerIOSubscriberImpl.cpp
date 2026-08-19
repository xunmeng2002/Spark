#include "ServerIOSubscriberImpl.h"
#include <Spark/Core/Logger/Logger.h>
#include <Spark/Core/Platform/Platform.h>
#include <string.h>
#include <iostream>

using namespace spark::core;
using namespace spark::network;

ServerIOSubscriberImpl::ServerIOSubscriberImpl(IOBase* io, IOThread* ioThread)
    :m_IO(io), m_IOThread(ioThread)
{
    m_IO->Subscribe(this);
}
ServerIOSubscriberImpl::~ServerIOSubscriberImpl()
{
    m_IO->UnSubscribe();
}

void ServerIOSubscriberImpl::OnConnect(SessionIDType sessionID, const char* ip, int port)
{
    WriteLog(LogLevel::Info, "ServerIOSubscriberImpl::OnConnect SessionID:[%lld], IP:[%s], Port:[%d]", sessionID, ip, port);
}
void ServerIOSubscriberImpl::OnDisConnect(SessionIDType sessionID, const char* ip, int port)
{
    WriteLog(LogLevel::Info, "ServerIOSubscriberImpl::OnDisConnect SessionID:[%lld], IP:[%s], Port:[%d]", sessionID, ip, port);
}



void ServerIOSubscriberImpl::OnRecv(SessionIDType sessionID, Buffer<BuffSize>* buffer)
{
    auto count = ++m_MessageCounts[sessionID];
    //if (count % 1000 == 0)
    {
        char message[2048] = { 0 };
        auto n = sprintf(message, "ServerIOSubscriberImpl::OnRecv SessionID:[%lld], Data:[%s]", (long long)sessionID, buffer->GetData());
        WriteLog(LogLevel::Info, message);
    }

    auto responseBuffer = new Buffer<BuffSize>();
    responseBuffer->Append(buffer->GetData(), buffer->GetLength());
    m_IO->Send(sessionID, responseBuffer);
}
