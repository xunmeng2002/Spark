#pragma once
#include <Spark/Network/NetworkExport.h>
#include <Spark/Types.h>
#include <Spark/Network/Io/Connect.h>
#include <Spark/TemplateLib/Buffer/ShmBuffer.h>
#include <Spark/TemplateLib/ObjectPool/ObjectPool.h>

namespace Spark::Network
{
template <unsigned SIZE> class ShmConnect : public Connect
{
public:
    ShmConnect(SessionIdType sessionId, const char* remoteAddress, int remotePort, ServerTypeType serverType, void* shmAddr,
               ConnectStatusType connectStatus)
        : Connect(sessionId, remoteAddress, remotePort, connectStatus)
    {
        shmBuffer_ = ShmBuffer<SIZE>::Allocate(serverType, remotePort, shmAddr, connectStatus);
    }
    virtual ~ShmConnect()
    {
        shmBuffer_->Deallocate();
        shmBuffer_ = nullptr;
    }

    static ShmConnect* Allocate(SessionIdType sessionId, const char* remoteAddress, int remotePort, ServerTypeType serverType, void* shmAddr,
                                ConnectStatusType connectStatus)
    {
        return ObjectPool<ShmConnect<SIZE>>::GetInstance().Allocate(sessionId, remoteAddress, remotePort, serverType, shmAddr, connectStatus);
    }
    virtual void Deallocate() override { ObjectPool<ShmConnect<SIZE>>::GetInstance().Deallocate(this); }
    ShmBuffer<SIZE>* GetBuffer() { return shmBuffer_; }

private:
    ShmBuffer<SIZE>* shmBuffer_;
};
}
