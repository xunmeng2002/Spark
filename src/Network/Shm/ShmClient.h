#pragma once
#include "Shm/ShmBase.h"

namespace Spark::Network
{
class ShmClient : public ShmBase
{
public:
    ShmClient(const char* shmName, int milliSeconds);
    virtual ~ShmClient();

    virtual bool ConnectToServer(const char* addressName) override;

protected:
    virtual void ConnectToServer() override;
    virtual void CheckConnect() override;
    virtual void CheckData() override;
    virtual void HandleData() override;

    void SendConnect();
    void CheckConnectResult();
    bool EstablishConfirmedConnection(size_t connectionIndex);
    virtual void RemoveConnect(Connect* connect) override;

    bool connected_;
    bool hasSendConnect_;
    ShmConnect<ShmBufferSize>* shmConnect_;
};
}
