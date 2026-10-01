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
                        if (!AddConnect(shmConnect))
                        {
                            DiscardRefusedConnect(shmConnect);
                            // 槽位 i 只被本函数短暂置成 Accepted、且从未经 DownWriteCount 告知客户端，须放回空闲：
                            // 析构里那道 CAS 只会把它推进 DisConnected，而全仓没有认领该状态的路径——Accept 只挑
                            // UnConnected、TryReclaimConnect 只遍历 connects_，被拒连接从未入表——不还原即永久失去该槽。
                            SingleShmHeader::ResetChannelHeader(commonShmHeader_ + i);
                            // 复用上面「槽位已满」的既有拒绝信号：客户端当场就能得到答复，不必干等握手超时。
                            SingleShmHeader::StoreStatus(commonShmHeader_, ConnectStatusType::Rejected);
                            break;
                        }

                        SingleShmHeader::StoreMappedField(commonShmHeader_->DownWriteCount, i);
                        SingleShmHeader::StoreStatus(commonShmHeader_, ConnectStatusType::Accepted);
                        ++connectCount_;
                        break;
                    }
                }
            }
            lastWriteTimePoint_ = chrono::steady_clock::now();
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
        auto currTimePoint = chrono::steady_clock::now();
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
    const auto currentTimePoint = chrono::steady_clock::now();
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

bool ShmServer::TryReclaimConnect(ShmConnect<ShmBufferSize>& shmConnect, const chrono::steady_clock::time_point& currentTimePoint)
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
