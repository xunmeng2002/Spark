#include "TestXtpClient.h"
#include "TestUtility/TestUtility.h"
#include "PackageFactory.h"
#include "Packages.h"
#include <Spark/Core/Utility/Utility.h>
#include <Spark/Core/Logger/Logger.h>
#include <Spark/Core/Utility/TimeUtility.h>
#include <Spark/Network/Io/IoThread.h>
#include <Spark/TemplateLib/ObjectPool/ObjectPool.h>

using namespace std;

using namespace Spark;
using namespace Spark::Core;
using namespace Spark::Network;
using namespace Spark::Packages;

XtpClient::XtpClient()
    : Protocol(ProtocolTypeType::Xtp, ServerTypeType::Client, IoModel, 0, new PackageFactory()), connected_(false), sessionId_(0LL), recvCount_(0)
{
    Subscribe(this);
    RegisterFront(TcpAddress);
}
XtpClient::~XtpClient() {}
void XtpClient::OnProtocolConnect(SessionIdType sessionId, const char* ip, int port)
{
    WriteLog(LogLevel::Info, "XtpClient::OnConnect SessionId:[%lld], IP:[%s], port:[%d]", sessionId, ip, port);

    sessionId_ = sessionId;
    connected_ = true;
    startTime_ = chrono::steady_clock::now();
    SendReqInsertOrder(++recvCount_);
}
void XtpClient::OnProtocolDisConnect(SessionIdType sessionId, const char* ip, int port)
{
    WriteLog(LogLevel::Info, "XtpClient::OnDisConnect SessionId:[%lld], IP:[%s], port:[%d]", sessionId, ip, port);

    connected_ = false;
    recvCount_ = 0;
}
void XtpClient::OnMessage(Package* package)
{
    ++recvCount_;
    //if (recvCount_ % 10000 == 0)
    {
        WriteLog(LogLevel::Info, "OnMessage: %s", package->GetDebugString());
    }
    if (recvCount_ < 10)
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

void XtpClient::SendReqInsertOrder(int index)
{
    // 与 StepClient 同理：包与其字段都按次领取、按次归还，不留在成员里跨次复用
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

void TestXtpClient()
{
    WriteLog(LogLevel::Info, "TestXtpClient");

    IoThread* ioThread = new IoThread("XtpClient");
    XtpClient xtpClient;
    xtpClient.SetIoThread(ioThread);
    if (!xtpClient.Init())
        return;
    ioThread->Start();
    ioThread->Join();
}
