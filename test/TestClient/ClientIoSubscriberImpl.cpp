#include "ClientIoSubscriberImpl.h"
#include "Packages.h"
#include <Spark/Core/Core.h>
#include <cstring>

using namespace std;
using namespace std::chrono;
using namespace Spark;
using namespace Spark::Core;
using namespace Spark::Network;
using namespace Spark::Packages;

ClientIoSubscriberImpl::ClientIoSubscriberImpl(IoBase* io, IoThread* ioThread) : io_(io), ioThread_(ioThread)
{
    io_->Subscribe(this);
}
ClientIoSubscriberImpl::~ClientIoSubscriberImpl()
{
    io_->UnSubscribe();
}

void ClientIoSubscriberImpl::OnConnect(SessionIdType sessionId, const char* ip, int port)
{
    WriteLog(LogLevel::Info, "ClientIoSubscriberImpl::OnConnect SessionId:[%lld], IP:[%s], Port:[%d]", sessionId, ip, port);
    messageCounts_.insert(std::make_pair(sessionId, 0));
    startSendTime_ = steady_clock::now();
    Send(sessionId);
}
void ClientIoSubscriberImpl::OnDisConnect(SessionIdType sessionId, const char* ip, int port)
{
    WriteLog(LogLevel::Info, "ClientIoSubscriberImpl::OnDisConnect SessionId:[%lld], IP:[%s], Port:[%d]", sessionId, ip, port);
    messageCounts_.erase(sessionId);

    ioThread_->Stop();
}
void ClientIoSubscriberImpl::OnRecv(SessionIdType sessionId, Buffer<BufferSize>* buffer)
{
    auto count = messageCounts_[sessionId];
    if (count % 100 == 0)
    {
        WriteLog(LogLevel::Info, "ClientIoSubscriberImpl::OnRecv SessionId:[%lld], Length:[%zu], Data:[%s]", sessionId, buffer->GetLength(),
                 buffer->GetData());
    }
    if (messageCounts_[sessionId] < 10000)
    {
        Send(sessionId);
    }
    else
    {
        auto duration = TimeUtility::GetDuration<milliseconds>(startSendTime_);
        WriteLog(LogLevel::Info, "TimeCost:%lld ms", duration);

        io_->DisConnect(sessionId);
        buffer->Deallocate();
    }
}
void ClientIoSubscriberImpl::Send(SessionIdType sessionId)
{
    auto count = ++messageCounts_[sessionId];

    ReqInsertOrderPackage reqInsertOrder;
    reqInsertOrder.ReqInsertOrder = new ReqInsertOrderField();
    Utility::Strcpy(reqInsertOrder.ReqInsertOrder->AccountId, "Xunmeng001");
    Utility::Strcpy(reqInsertOrder.ReqInsertOrder->ExchangeId, "SHSE");
    Utility::Strcpy(reqInsertOrder.ReqInsertOrder->InstrumentId, "600036");
    reqInsertOrder.ReqInsertOrder->Direction = DirectionType::Buy;
    reqInsertOrder.ReqInsertOrder->OffsetFlag = OffsetFlagType::Open;
    reqInsertOrder.ReqInsertOrder->OrderPriceType = OrderPriceTypeType::LimitPrice;
    reqInsertOrder.ReqInsertOrder->Price = 88.88;
    reqInsertOrder.ReqInsertOrder->Volume = 1000;
    reqInsertOrder.ReqInsertOrder->ClientOrderId = count;
    auto message = reqInsertOrder.GetDebugString();

    Buffer<BufferSize>* buffer = Buffer<BufferSize>::Allocate();
    auto data = buffer->GetData();
    auto len = strlen(message);
    memcpy(data, message, len);
    buffer->SetLength(len);

    io_->Send(sessionId, buffer);
}
void ClientIoSubscriberImpl::SendCommand(SessionIdType sessionId, const char* cmd)
{
    ++messageCounts_[sessionId];
    Buffer<BufferSize>* buffer = Buffer<BufferSize>::Allocate();
    int n = sprintf(buffer->GetData(), "%s\r\n", cmd);
    buffer->SetLength(n);
    io_->Send(sessionId, buffer);
}
