#pragma once
#include <Spark/Network/NetworkExport.h>
#include <Spark/Types.h>
#include <Spark/Network/Io/Connect.h>
#include <Spark/TemplateLib/Buffer/ShmBuffer.h>
#include <Spark/TemplateLib/ObjectPool/ObjectPool.h>

namespace Spark::Network
{
constexpr unsigned int ShmBufferSize = 1024 * 1024;

template <unsigned Size>
class ShmConnect : public Connect
{
public:
    ShmConnect(SessionIdType sessionId, const char* remoteAddress, int remotePort, ServerTypeType serverType, void* shmAddr,
               ConnectStatusType connectStatus)
        : Connect(sessionId, remoteAddress, remotePort, connectStatus)
    {
        shmBuffer_ = ShmBuffer<Size>::Allocate(serverType, remotePort, shmAddr, connectStatus);
    }
    virtual ~ShmConnect()
    {
        shmBuffer_->Deallocate();
        shmBuffer_ = nullptr;
    }

    static ShmConnect* Allocate(SessionIdType sessionId, const char* remoteAddress, int remotePort, ServerTypeType serverType, void* shmAddr,
                                ConnectStatusType connectStatus)
    {
        return ObjectPool<ShmConnect<Size>>::GetInstance().Allocate(sessionId, remoteAddress, remotePort, serverType, shmAddr, connectStatus);
    }
    virtual void Deallocate() override { ObjectPool<ShmConnect<Size>>::GetInstance().Deallocate(this); }
    ShmBuffer<Size>* GetBuffer() { return shmBuffer_; }

private:
    ShmBuffer<Size>* shmBuffer_;
};
}
