#include "Shm/ShmClient.h"
#include <Spark/Core/Logger/Logger.h>
#include <string.h>

using namespace std;

using namespace Spark::Core;
namespace Spark::Network
{
ShmClient::ShmClient(const char* shmName, int milliSeconds)
    : ShmBase(ServerTypeType::Client, shmName, milliSeconds), connected_(false), hasSendConnect_(false), shmConnect_(nullptr)
{
}
ShmClient::~ShmClient()
{
    if (shmConnect_ != nullptr)
    {
        RemoveConnect(shmConnect_);
    }
}
bool ShmClient::ConnectToServer(const char* addressName)
{
    string address, port;
    ParseAddress(addressName, address, port);
    if (address == address_ && connected_)
    {
        return true;
    }
    if (address != address_)
    {
        if (shmConnect_ != nullptr)
        {
            RemoveConnect(shmConnect_);
        }
        address_ = address;
        port_ = port;
        connected_ = false;
        hasSendConnect_ = false;
        shmConnect_ = nullptr;
        WriteLog(LogLevel::Info, "Address Changed. Address:%s Port:%s\n", address_.c_str(), port_.c_str());
        if (!Init())
            return false;
    }
    ConnectToServer();
    while (hasSendConnect_ && SingleShmHeader::LoadStatus(commonShmHeader_) == ConnectStatusType::Connecting)
    {
        this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return connected_;
}
void ShmClient::ConnectToServer()
{
    if (connected_)
        return;
    if (!hasSendConnect_ && SingleShmHeader::LoadStatus(commonShmHeader_) == ConnectStatusType::UnConnected)
    {
        SendConnect();
    }
    else if (hasSendConnect_ && SingleShmHeader::LoadStatus(commonShmHeader_) != ConnectStatusType::Connecting)
    {
        CheckConnectResult();
    }
}
void ShmClient::CheckConnect()
{
    if (!connected_)
        return;
    if (shmConnect_->GetBuffer()->GetConnectStatus() == ConnectStatusType::DisConnected)
    {
        RemoveConnect(shmConnect_);
    }
}
void ShmClient::CheckData()
{
    if (!connected_)
        return;
    if (shmConnect_->GetBuffer()->GetReadBufferSize() > 0)
        return;
    sems_[shmConnect_->RemotePort]->Lock();
}
void ShmClient::HandleData()
{
    if (!connected_)
        return;
    if (shmConnect_->GetBuffer()->GetReadBufferSize() > 0)
    {
        DoRecv(shmConnect_);
    }
}

void ShmClient::SendConnect()
{
    if (semConnect_->Lock())
    {
        if (SingleShmHeader::LoadStatus(commonShmHeader_) == ConnectStatusType::UnConnected)
        {
            SingleShmHeader::StoreStatus(commonShmHeader_, ConnectStatusType::Connecting);
            hasSendConnect_ = true;
        }
        semConnect_->UnLock();
    }
    else
    {
        WriteLog(LogLevel::Info, "Sem Lock Failed. Sleep 10ms\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
void ShmClient::CheckConnectResult()
{
    if (semConnect_->Lock())
    {
        hasSendConnect_ = false;
        const ConnectStatusType connectResultStatus = SingleShmHeader::LoadStatus(commonShmHeader_);
        if (connectResultStatus == ConnectStatusType::Accepted)
        {
            const auto index = SingleShmHeader::LoadMappedField(commonShmHeader_->DownWriteCount);
            if (ShmBuffer<ShmBufferSize>::IsConnectionIndexWithinMapping(index, maxConnectSize_))
            {
                if (SingleShmHeader::ConfirmAcceptedConnection(commonShmHeader_ + index))
                {
                    shmConnect_ = ShmConnect<ShmBufferSize>::Allocate(GetSessionId(), address_.c_str(), static_cast<int>(index), serverType_,
                                                                      shmAddr_, ConnectStatusType::Connected, maxConnectSize_);
                    AddConnect(shmConnect_);
                    connected_ = true;
                }
                else
                {
                    WriteLog(LogLevel::Warning, "Confirm Connect Revoked. Index:%zu, Address:%s", index, address_.c_str());
                }
            }
            else
            {
                WriteLog(LogLevel::Warning, "Reject Connect Index:%zu Out Of Range, ConnectSize:%u, Address:%s", index, maxConnectSize_,
                         address_.c_str());
            }
            SingleShmHeader::StoreStatus(commonShmHeader_, ConnectStatusType::UnConnected);
        }
        else if (connectResultStatus == ConnectStatusType::Rejected)
        {
            SingleShmHeader::StoreStatus(commonShmHeader_, ConnectStatusType::UnConnected);
        }
        else
        {
            WriteLog(LogLevel::Info, "UnExpected Status:%d\n", static_cast<int>(connectResultStatus));
        }
        semConnect_->UnLock();
        if (!connected_)
        {
            WriteLog(LogLevel::Info, "Connect Failed. Sleep 1s\n");
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    else
    {
        WriteLog(LogLevel::Info, "Sem Lock Failed. Sleep 10ms\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
void ShmClient::RemoveConnect(Connect* connect)
{
    ShmBase::RemoveConnect(connect);
    connected_ = false;
    hasSendConnect_ = false;
    shmConnect_ = nullptr;
}
}
