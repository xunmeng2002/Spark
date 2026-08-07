#include "ClientIOSubscriberImpl.h"
#include "Packages.h"
#include <Spark/Core/Core.h>
#include <cstring>


using namespace std;
using namespace std::chrono;

ClientIOSubscriberImpl::ClientIOSubscriberImpl(IOBase* io, IOThread* ioThread)
    :m_IO(io), m_IOThread(ioThread)
{
    m_IO->Subscribe(this);
}
ClientIOSubscriberImpl::~ClientIOSubscriberImpl()
{
    m_IO->UnSubscribe();
}


void ClientIOSubscriberImpl::OnConnect(SessionIDType sessionID, const char* ip, int port)
{
    WriteLog(LogLevel::Info, "ClientIOSubscriberImpl::OnConnect SessionID:[%lld], IP:[%s], Port:[%d]", sessionID, ip, port);
    m_MessageCounts.insert(std::make_pair(sessionID, 0));
    m_StartSendTime = steady_clock::now();
    Send(sessionID);
}
void ClientIOSubscriberImpl::OnDisConnect(SessionIDType sessionID, const char* ip, int port)
{
    WriteLog(LogLevel::Info, "ClientIOSubscriberImpl::OnDisConnect SessionID:[%lld], IP:[%s], Port:[%d]", sessionID, ip, port);
    m_MessageCounts.erase(sessionID);

    m_IOThread->Stop();
}
void ClientIOSubscriberImpl::OnRecv(SessionIDType sessionID, Buffer<BuffSize>* buffer)
{
    auto count = m_MessageCounts[sessionID];
    if (count % 100 == 0)
    {
        WriteLog(LogLevel::Info, "ClientIOSubscriberImpl::OnRecv SessionID:[%lld], Length:[%d], Data:[%s]", sessionID, buffer->GetLength(), buffer->GetData());
    }
    if (m_MessageCounts[sessionID] < 10000)
    {
        Send(sessionID);
    }
    else
    {
        auto duration = TimeUtility::GetDuration<milliseconds>(m_StartSendTime);
        WriteLog(LogLevel::Info, "TimeCost:%lld ms", duration);

        m_IO->DisConnect(sessionID);
        buffer->Deallocate();
    }
}
void ClientIOSubscriberImpl::Send(SessionIDType sessionID)
{
    auto count = ++m_MessageCounts[sessionID];

    ReqInsertOrderPackage reqInsertOrder;
    reqInsertOrder.ReqInsertOrder = new ReqInsertOrderField();
    Utility::Strcpy(reqInsertOrder.ReqInsertOrder->AccountID, "Xunmeng001");
    Utility::Strcpy(reqInsertOrder.ReqInsertOrder->ExchangeID, "SHSE");
    Utility::Strcpy(reqInsertOrder.ReqInsertOrder->InstrumentID, "600036");
    reqInsertOrder.ReqInsertOrder->Direction = DirectionType::Buy;
    reqInsertOrder.ReqInsertOrder->OffsetFlag = OffsetFlagType::Open;
    reqInsertOrder.ReqInsertOrder->OrderPriceType = OrderPriceTypeType::LimitPrice;
    reqInsertOrder.ReqInsertOrder->Price = 88.88;
    reqInsertOrder.ReqInsertOrder->Volume = 1000;
    reqInsertOrder.ReqInsertOrder->ClientOrderID = count;
    auto message = reqInsertOrder.GetDebugString();

    Buffer<BuffSize>* buffer = Buffer<BuffSize>::Allocate();
    auto data = buffer->GetData();
    auto len = strlen(message);
    memcpy(data, message, len);
    buffer->SetLength((unsigned)len);

    m_IO->Send(sessionID, buffer);
}
void ClientIOSubscriberImpl::SendCommand(SessionIDType sessionID, const char* cmd)
{
    ++m_MessageCounts[sessionID];
    Buffer<BuffSize>* buffer = Buffer<BuffSize>::Allocate();
    int n = sprintf(buffer->GetData(), "%s\r\n", cmd);
    buffer->SetLength(n);
    m_IO->Send(sessionID, buffer);
}

