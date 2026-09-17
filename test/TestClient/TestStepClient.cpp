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
    :Protocol(ProtocolTypeType::Step, ServerTypeType::Client, IoModel, 0, new PackageFactory()), connected_(false), sessionId_(0LL), recvCount_(0)
{
    reqInsertOrder_ = new ReqInsertOrderPackage();
    Subscribe(this);
    RegisterFront(TcpAddress);
}
StepClient::~StepClient()
{
}

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
void StepClient::OnMessage(Package* package)
{
    if (++recvCount_ % 10000 == 0)
    {
        WriteLog(LogLevel::Info, "OnMessage: %s", package->GetDebugString());
    }
    if (recvCount_ < 1000000)
    {
        SendReqInsertOrder(recvCount_);
    }
    else
    {
        WriteLog(LogLevel::Info, "OnMessage: %s", package->GetDebugString());
        WriteLog(LogLevel::Info, "Total Cost: %lld ms", TimeUtility::GetDuration<chrono::milliseconds>(startTime_));
        ioThread_->Stop();
    }
}
void StepClient::SendReqInsertOrder(int index)
{
    reqInsertOrder_->Prepare(sessionId_, false, index);
    reqInsertOrder_->ReqInsertOrder = ObjectPool<ReqInsertOrderField>::GetInstance().Allocate();
    memset(reqInsertOrder_->ReqInsertOrder, 0, sizeof(ReqInsertOrderField));
    Utility::Strcpy(reqInsertOrder_->ReqInsertOrder->AccountId, "Xunmeng001");
    Utility::Strcpy(reqInsertOrder_->ReqInsertOrder->ExchangeId, "SHSE");
    Utility::Strcpy(reqInsertOrder_->ReqInsertOrder->InstrumentId, "600036");
    reqInsertOrder_->ReqInsertOrder->Direction = DirectionType::Buy;
    reqInsertOrder_->ReqInsertOrder->OffsetFlag = OffsetFlagType::Open;
    reqInsertOrder_->ReqInsertOrder->OrderPriceType = OrderPriceTypeType::LimitPrice;
    reqInsertOrder_->ReqInsertOrder->Price = 100 + index;
    reqInsertOrder_->ReqInsertOrder->Volume = index;
    reqInsertOrder_->ReqInsertOrder->ClientOrderId = index;
    Send(reqInsertOrder_);
    reqInsertOrder_->Deallocate();
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



