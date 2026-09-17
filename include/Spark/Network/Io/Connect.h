#pragma once
#include <Spark/Network/NetworkExport.h>
#include <Spark/Types.h>
#include <Spark/TemplateLib/Buffer/Buffer.h>
#include <list>
#include <mutex>

namespace Spark::Network
{
class NETWORK_EXPORTS Connect
{
public:
    Connect(SessionIdType sessionId, const char* remoteAddress, int remotePort, ConnectStatusType connectStatus)
        : SessionId(sessionId), ConnectStatus(connectStatus), RemotePort(remotePort)
    {
        strncpy(RemoteAddress, remoteAddress, sizeof(RemoteAddress));
    }
    virtual ~Connect() = default;

    virtual void Deallocate() = 0;
    void PushBack(Buffer<BuffSize>* buffer);
    void PushFront(Buffer<BuffSize>* buffer);
    Buffer<BuffSize>* GetNextBuffer();

    SessionIdType SessionId = 0LL;
    ConnectStatusType ConnectStatus = ConnectStatusType::UnConnected;
    char RemoteAddress[40]{0};
    int RemotePort = 0;
    std::list<Buffer<BuffSize>*> Buffers;
    std::mutex BuffersMutex;
};
}
