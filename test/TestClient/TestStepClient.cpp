#include "TestStepClient.h"
#include "TestUtility/TestUtility.h"
#include "PackageFactory.h"
#include "Packages.h"
#include <Spark/Core/Utility/Utility.h>
#include <Spark/Core/Utility/TimeUtility.h>
#include <Spark/Core/Logger/Logger.h>
#include <Spark/Network/Io/IoThread.h>
#include <Spark/TemplateLib/ObjectPool/ObjectPool.h>

using namespace std;
using namespace Spark;
using namespace Spark::Core;
using namespace Spark::Network;
using namespace Spark::Packages;

StepClient::StepClient()
    : Protocol(ProtocolTypeType::Step, ServerTypeType::Client, IoModel, 0, new PackageFactory()), connected_(false), sessionId_(0LL), recvCount_(0)
{
    Subscribe(this);
    RegisterFront(TcpAddress);
}
StepClient::~StepClient() {}

void StepClient::OnProtocolConnect(SessionIdType sessionId, const char* ip, int port)
{
    WriteLog(LogLevel::Info, "StepClient::OnConnect SessionId:[%lld], IP:[%s], port:[%d]", sessionId, ip, port);

    sessionId_ = sessionId;
    connected_ = true;
    startTime_ = chrono::steady_clock::now();
    SendReqInsertOrder(++recvCount_);
}
void StepClient::OnProtocolDisConnect(SessionIdType sessionId, const char* ip, int port)
{
    WriteLog(LogLevel::Info, "StepClient::OnDisConnect SessionId:[%lld], IP:[%s], port:[%d]", sessionId, ip, port);

    connected_ = false;
    recvCount_ = 0;
}
void StepClient::OnMessage(Package* ownedPackage)
{
    if (++recvCount_ % 10000 == 0)
    {
        WriteLog(LogLevel::Info, "OnMessage: %s", ownedPackage->GetDebugString());
    }
    if (recvCount_ < 1000000)
    {
        SendReqInsertOrder(recvCount_);
    }
    else
    {
        WriteLog(LogLevel::Info, "OnMessage: %s", ownedPackage->GetDebugString());
        WriteLog(LogLevel::Info, "Total Cost: %lld ms", TimeUtility::GetDuration<chrono::milliseconds>(startTime_));
        ioThread_->Stop();
    }
    ownedPackage->Deallocate();
}
void StepClient::SendReqInsertOrder(int index)
{
    // 池只认自己发出去的指针：本对象与其中的字段都必须在本次调用内成对领取/归还，
    // 故按次领一个包而非常驻成员——常驻成员归还后再用，既是池契约下的未定义行为，
    // 也会把同一个指针重复推入空闲链
    ReqInsertOrderPackage* reqInsertOrder = ReqInsertOrderPackage::Allocate();
    reqInsertOrder->Prepare(sessionId_, false, index);
    reqInsertOrder->ReqInsertOrder = ObjectPool<ReqInsertOrderField>::GetInstance().Allocate();
    memset(reqInsertOrder->ReqInsertOrder, 0, sizeof(ReqInsertOrderField));
    Utility::Strcpy(reqInsertOrder->ReqInsertOrder->AccountId, "Xunmeng001");
    Utility::Strcpy(reqInsertOrder->ReqInsertOrder->ExchangeId, "SHSE");
    Utility::Strcpy(reqInsertOrder->ReqInsertOrder->InstrumentId, "600036");
    reqInsertOrder->ReqInsertOrder->Direction = DirectionType::Buy;
    reqInsertOrder->ReqInsertOrder->OffsetFlag = OffsetFlagType::Open;
    reqInsertOrder->ReqInsertOrder->OrderPriceType = OrderPriceTypeType::LimitPrice;
    reqInsertOrder->ReqInsertOrder->Price = 100 + index;
    reqInsertOrder->ReqInsertOrder->Volume = index;
    reqInsertOrder->ReqInsertOrder->ClientOrderId = index;
    Send(reqInsertOrder);
    reqInsertOrder->Deallocate();
}

void TestStepClient()
{
    WriteLog(LogLevel::Info, "TestStepClient");

    IoThread* ioThread = new IoThread("StepClient");
    StepClient stepClient;
    stepClient.SetIoThread(ioThread);
    if (!stepClient.Init())
        return;
    ioThread->Start();
    ioThread->Join();
}
