#pragma once
#include <Spark/Network/NetworkExport.h>
#include <Spark/Types.h>
#include <Spark/Network/Io/Connect.h>
#include <Spark/TemplateLib/Buffer/ShmBuffer.h>
#include <Spark/TemplateLib/ObjectPool/ObjectPool.h>

#include <chrono>
#include <cstddef>

namespace Spark::Network
{
constexpr unsigned int ShmBufferSize = 1024 * 1024;

template <size_t Size>
class ShmConnect : public Connect
{
public:
    ShmConnect(SessionIdType sessionId, const char* remoteAddress, int remotePort, ServerTypeType serverType, void* shmAddr,
               ConnectStatusType connectStatus, unsigned connectionCount)
        : Connect(sessionId, remoteAddress, remotePort, connectStatus), CreateTimePoint(std::chrono::steady_clock::now())
    {
        shmBuffer_ = ObjectPool<ShmBuffer<Size>>::GetInstance().Allocate(serverType, remotePort, shmAddr, connectStatus, connectionCount);
    }
    virtual ~ShmConnect()
    {
        shmBuffer_->MarkDisconnectedAndResetChannelWhenLastHolder();
        ObjectPool<ShmBuffer<Size>>::GetInstance().Deallocate(shmBuffer_);
        shmBuffer_ = nullptr;
    }

    static ShmConnect* Allocate(SessionIdType sessionId, const char* remoteAddress, int remotePort, ServerTypeType serverType, void* shmAddr,
                                ConnectStatusType connectStatus, unsigned connectionCount)
    {
        return ObjectPool<ShmConnect<Size>>::GetInstance().Allocate(sessionId, remoteAddress, remotePort, serverType, shmAddr, connectStatus,
                                                                    connectionCount);
    }
    virtual void Deallocate() override { ObjectPool<ShmConnect<Size>>::GetInstance().Deallocate(this); }
    ShmBuffer<Size>* GetBuffer() { return shmBuffer_; }

    std::chrono::steady_clock::time_point CreateTimePoint;

private:
    ShmBuffer<Size>* shmBuffer_;
};
}
