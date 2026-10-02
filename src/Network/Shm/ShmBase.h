#pragma once
#include "Shm/Sem.h"
#include "Shm/ShmConnect.h"
#include <Spark/Types.h>
#include <Spark/TemplateLib/Buffer/LinearBuffer.h>
#include <Spark/Network/Io/IoBase.h>
#include <string>
#include <map>
#include <list>
#include <cstddef>
#include <limits>
#include <vector>
#include <mutex>

namespace Spark::Network
{
class ShmBase : public IoBase
{
public:
    ShmBase(ServerTypeType shmType, const char* shmName, int milliSeconds);
    virtual ~ShmBase();

    virtual bool Init() override;
    virtual void Send(SessionIdType sessionId, LinearBuffer<BufferSize>* buffer) override;

    virtual void HandleIoEvent() override;

protected:
    virtual void DoSend(Connect* connect) override;
    virtual void DoRecv(Connect* connect) override;
    virtual void AddConnect(Connect* connect) override;

    virtual void ConnectToServer() {}
    virtual void Accept() {}
    virtual void CheckConnect() = 0;
    virtual void CheckData() = 0;
    virtual void HandleData() = 0;

    std::string shmName_;
    unsigned maxConnectSize_;
    SingleShmHeader* commonShmHeader_;
    void* shmAddr_;
    Sem* semConnect_;
    std::vector<Sem*> sems_;

#ifdef _WIN32
    void* file_;
    void* fileMap_;
#endif // _WIN32

private:
    static constexpr int SendBlockedWarningSeconds = 1;

    static constexpr unsigned MaxSharedMemoryConnectSize = (std::numeric_limits<unsigned>::max)() / (ShmBufferSize * 2);
    static constexpr size_t ConnectionZeroRegionSize = static_cast<size_t>(ShmBufferSize) * 2;

    static_assert(MaxSharedMemoryConnectSize * sizeof(SingleShmHeader) <= ConnectionZeroRegionSize,
                  "SingleShmHeader array must fit in the region reserved for connection zero");

    bool IsConnectSizeAllowed() const;
    bool IsReusedMappingLayoutCompatible() const;
    bool WindowsInit();
    bool LinuxInit();
    unsigned GetSharedMemoryMappingSize() const;

    bool reusedExistingShmObject_;
    bool createdShmObjectInThisInit_;
};
}
