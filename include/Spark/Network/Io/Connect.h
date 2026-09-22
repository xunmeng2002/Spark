#pragma once
#include <Spark/Network/NetworkExport.h>
#include <Spark/Types.h>
#include <Spark/TemplateLib/Buffer/Buffer.h>
#include <list>
#include <mutex>

namespace Spark::Network
{
constexpr unsigned int BufferSize = 64 * 1024;

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
    void PushBack(Buffer<BufferSize>* buffer);
    void PushFront(Buffer<BufferSize>* buffer);
    Buffer<BufferSize>* GetNextBuffer();

    SessionIdType SessionId = 0LL;
    ConnectStatusType ConnectStatus = ConnectStatusType::UnConnected;
    char RemoteAddress[40]{0};
    int RemotePort = 0;
    std::list<Buffer<BufferSize>*> Buffers;
    std::mutex BuffersMutex;
};
}
