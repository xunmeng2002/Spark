#include "Shm/ShmServer.h"
#include <Spark/Core/Utility/TimeUtility.h>
#include <string.h>

using namespace std;

namespace Spark::Network
{
ShmServer::ShmServer(const char* shmName, int milliSeconds) : ShmBase(ServerTypeType::Server, shmName, milliSeconds), connectCount_(0) {}
ShmServer::~ShmServer() {}
void ShmServer::Accept()
{
    switch (SingleShmHeader::LoadStatus(commonShmHeader_))
    {
    case ConnectStatusType::UnConnected:
        break;
    case ConnectStatusType::Connecting:
    {
        if (semConnect_->Lock())
        {
            if (connectCount_ >= maxConnectSize_ - 1)
            {
                SingleShmHeader::StoreStatus(commonShmHeader_, ConnectStatusType::Rejected);
            }
            else
            {
                for (auto i = 1U; i < maxConnectSize_; ++i)
                {
                    auto shmHeader = commonShmHeader_ + i;
                    if (SingleShmHeader::LoadStatus(shmHeader) == ConnectStatusType::UnConnected)
                    {
                        ShmConnect<ShmBufferSize>* shmConnect = ShmConnect<ShmBufferSize>::Allocate(
                            GetSessionId(), address_.c_str(), i, serverType_, shmAddr_, ConnectStatusType::Accepted, maxConnectSize_);
                        AddConnect(shmConnect);

                        SingleShmHeader::StoreMappedField(commonShmHeader_->DownWriteCount, i);
                        SingleShmHeader::StoreStatus(commonShmHeader_, ConnectStatusType::Accepted);
                        ++connectCount_;
                        break;
                    }
                }
            }
            lastWriteTimePoint_ = chrono::system_clock::now();
            semConnect_->UnLock();
        }
        else
        {
            printf("Sem Lock Failed.\n");
        }
        break;
    }
    case ConnectStatusType::Accepted:
    case ConnectStatusType::Rejected:
    {
        auto currTimePoint = chrono::system_clock::now();
        auto t = chrono::duration_cast<chrono::seconds>(currTimePoint - lastWriteTimePoint_);
        if (t.count() >= HandshakeTimeoutSeconds)
        {
            if (semConnect_->Lock())
            {
                const ConnectStatusType timedOutStatus = SingleShmHeader::LoadStatus(commonShmHeader_);
                if (timedOutStatus == ConnectStatusType::Accepted || timedOutStatus == ConnectStatusType::Rejected)
                {
                    printf("Reset Connect From Server,  Status:%d\n", static_cast<int>(timedOutStatus));
                    SingleShmHeader::StoreStatus(commonShmHeader_, ConnectStatusType::UnConnected);
                }
                semConnect_->UnLock();
            }
            else
            {
                printf("Sem Lock Failed.\n");
            }
        }
    }
    break;
    case ConnectStatusType::Connected:
    case ConnectStatusType::DisConnected:
        break;
    default:
        break;
    }
}
void ShmServer::CheckConnect()
{
    const auto currentTimePoint = chrono::system_clock::now();
    for (auto& it : connects_)
    {
        auto shmConnect = static_cast<ShmConnect<ShmBufferSize>*>(it.second);
        if (TryReclaimConnect(*shmConnect, currentTimePoint))
        {
            lock_guard<mutex> guard(disConnectSessionIdsMutex_);
            disConnectSessionIds_.push_back(shmConnect->SessionId);
        }
    }
}

bool ShmServer::TryReclaimConnect(ShmConnect<ShmBufferSize>& shmConnect, const chrono::system_clock::time_point& currentTimePoint)
{
    const ConnectStatusType connectStatus = shmConnect.GetBuffer()->GetConnectStatus();
    if (connectStatus == ConnectStatusType::DisConnected)
    {
        return true;
    }
    if (connectStatus != ConnectStatusType::Accepted)
    {
        return false;
    }
    if (chrono::duration_cast<chrono::seconds>(currentTimePoint - shmConnect.CreateTimePoint).count() < HandshakeTimeoutSeconds)
    {
        return false;
    }
    return shmConnect.GetBuffer()->RevokeUnconfirmedAccept();
}
void ShmServer::CheckData()
{
    for (auto& it : connects_)
    {
        auto shmConnect = static_cast<ShmConnect<ShmBufferSize>*>(it.second);
        if (shmConnect->GetBuffer()->GetReadBufferSize() > 0)
            return;
    }
    sems_[0]->Lock();
}
void ShmServer::HandleData()
{
    for (auto& it : connects_)
    {
        auto shmConnect = static_cast<ShmConnect<ShmBufferSize>*>(it.second);
        if (shmConnect->GetBuffer()->GetReadBufferSize() > 0)
        {
            DoRecv(shmConnect);
        }
    }
}

void ShmServer::RemoveConnect(Connect* connect)
{
    ShmBase::RemoveConnect(connect);
    --connectCount_;
}
}
