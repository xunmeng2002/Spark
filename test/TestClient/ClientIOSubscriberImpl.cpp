#include "ClientIOSubscriberImpl.h"
#include "Packages.h"
#include <Spark/Core/Core.h>
#include <cstring>


using namespace std;
using namespace std::chrono;
using namespace Spark;
using namespace Spark::Core;
using namespace Spark::Network;
using namespace Spark::Packages;

ClientIOSubscriberImpl::ClientIOSubscriberImpl(IOBase* io, IOThread* ioThread)
    :io_(io), ioThread_(ioThread)
{
    io_->Subscribe(this);
}
ClientIOSubscriberImpl::~ClientIOSubscriberImpl()
{
    io_->UnSubscribe();
}


void ClientIOSubscriberImpl::OnConnect(SessionIdType sessionId, const char* ip, int port)
{
    WriteLog(LogLevel::Info, "ClientIOSubscriberImpl::OnConnect SessionId:[%lld], IP:[%s], Port:[%d]", sessionId, ip, port);
    m_MessageCounts.insert(std::make_pair(sessionId, 0));
    m_StartSendTime = steady_clock::now();
    Send(sessionId);
}
void ClientIOSubscriberImpl::OnDisConnect(SessionIdType sessionId, const char* ip, int port)
{
    WriteLog(LogLevel::Info, "ClientIOSubscriberImpl::OnDisConnect SessionId:[%lld], IP:[%s], Port:[%d]", sessionId, ip, port);
    m_MessageCounts.erase(sessionId);

    ioThread_->Stop();
}
void ClientIOSubscriberImpl::OnRecv(SessionIdType sessionId, Buffer<BuffSize>* buffer)
{
    auto count = m_MessageCounts[sessionId];
    if (count % 100 == 0)
    {
        WriteLog(LogLevel::Info, "ClientIOSubscriberImpl::OnRecv SessionId:[%lld], Length:[%d], Data:[%s]", sessionId, buffer->GetLength(), buffer->GetData());
    }
    if (m_MessageCounts[sessionId] < 10000)
    {
        Send(sessionId);
    }
    else
    {
        auto duration = TimeUtility::GetDuration<milliseconds>(m_StartSendTime);
        WriteLog(LogLevel::Info, "TimeCost:%lld ms", duration);

        io_->DisConnect(sessionId);
        buffer->Deallocate();
    }
}
void ClientIOSubscriberImpl::Send(SessionIdType sessionId)
{
    auto count = ++m_MessageCounts[sessionId];

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

    Buffer<BuffSize>* buffer = Buffer<BuffSize>::Allocate();
    auto data = buffer->GetData();
    auto len = strlen(message);
    memcpy(data, message, len);
    buffer->SetLength((unsigned)len);

    io_->Send(sessionId, buffer);
}
void ClientIOSubscriberImpl::SendCommand(SessionIdType sessionId, const char* cmd)
{
    ++m_MessageCounts[sessionId];
    Buffer<BuffSize>* buffer = Buffer<BuffSize>::Allocate();
    int n = sprintf(buffer->GetData(), "%s\r\n", cmd);
    buffer->SetLength(n);
    io_->Send(sessionId, buffer);
}

