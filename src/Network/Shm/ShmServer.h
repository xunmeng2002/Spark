#pragma once
#include "Shm/ShmBase.h"
#include <chrono>

namespace Spark::Network
{
class ShmServer : public ShmBase
{
public:
    ShmServer(const char* shmName, int milliSeconds);
    ~ShmServer();

protected:
    static constexpr int HandshakeTimeoutSeconds = 5;

    unsigned connectCount_;
    std::chrono::system_clock::time_point lastWriteTimePoint_;

private:
    virtual void Accept() override;
    virtual void CheckConnect() override;
    virtual void CheckData() override;
    virtual void HandleData() override;

    virtual void RemoveConnect(Connect* connect) override;

    bool TryReclaimConnect(ShmConnect<ShmBufferSize>& shmConnect, const std::chrono::system_clock::time_point& currentTimePoint);
};
}
