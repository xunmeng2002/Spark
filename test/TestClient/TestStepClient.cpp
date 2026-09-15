#include "TestStepClient.h"
#include "TestUtility/TestUtility.h"
#include "PackageFactory.h"
#include "Packages.h"
#include <Spark/Core/Utility/Utility.h>
#include <Spark/Core/Utility/TimeUtility.h>
#include <Spark/Core/Logger/Logger.h>
#include <Spark/Network/IO/IOThread.h>
#include <Spark/TemplateLib/ObjectPool/ObjectPool.h>

using namespace std;
using namespace Spark;
using namespace Spark::Core;
using namespace Spark::Network;
using namespace Spark::Packages;

StepClient::StepClient()
	:Protocol(ProtocolTypeType::Step, ServerTypeType::Client, g_IOModel, 0, new PackageFactory()), connected_(false), sessionId_(0LL), m_RecvCount(0)
{
	m_ReqInsertOrder = new ReqInsertOrderPackage();
	Subscribe(this);
	RegisterFront(g_Address);
}
StepClient::~StepClient()
{
}

void StepClient::OnProtocolConnect(SessionIDType sessionID, const char* ip, int port)
{
	WriteLog(LogLevel::Info, "StepClient::OnConnect SessionID:[%lld], IP:[%s], port:[%d]", sessionID, ip, port);

	sessionId_ = sessionID;
	connected_ = true;
	m_StartTime = chrono::steady_clock::now();
	SendReqInsertOrder(++m_RecvCount);
}
void StepClient::OnProtocolDisConnect(SessionIDType sessionID, const char* ip, int port)
{
	WriteLog(LogLevel::Info, "StepClient::OnDisConnect SessionID:[%lld], IP:[%s], port:[%d]", sessionID, ip, port);

	connected_ = false;
	m_RecvCount = 0;
}
void StepClient::OnMessage(Package* package)
{
	if (++m_RecvCount % 10000 == 0)
	{
		WriteLog(LogLevel::Info, "OnMessage: %s", package->GetDebugString());
	}
	if (m_RecvCount < 1000000)
	{
		SendReqInsertOrder(m_RecvCount);
	}
	else
	{
		WriteLog(LogLevel::Info, "OnMessage: %s", package->GetDebugString());
		WriteLog(LogLevel::Info, "Total Cost: %lld ms", TimeUtility::GetDuration<chrono::milliseconds>(m_StartTime));
		ioThread_->Stop();
	}
}
void StepClient::SendReqInsertOrder(int index)
{
	m_ReqInsertOrder->Prepare(sessionId_, false, index);
	m_ReqInsertOrder->ReqInsertOrder = ObjectPool<ReqInsertOrderField>::GetInstance().Allocate();
	memset(m_ReqInsertOrder->ReqInsertOrder, 0, sizeof(ReqInsertOrderField));
	Utility::Strcpy(m_ReqInsertOrder->ReqInsertOrder->AccountID, "Xunmeng001");
	Utility::Strcpy(m_ReqInsertOrder->ReqInsertOrder->ExchangeID, "SHSE");
	Utility::Strcpy(m_ReqInsertOrder->ReqInsertOrder->InstrumentID, "600036");
	m_ReqInsertOrder->ReqInsertOrder->Direction = DirectionType::Buy;
	m_ReqInsertOrder->ReqInsertOrder->OffsetFlag = OffsetFlagType::Open;
	m_ReqInsertOrder->ReqInsertOrder->OrderPriceType = OrderPriceTypeType::LimitPrice;
	m_ReqInsertOrder->ReqInsertOrder->Price = 100 + index;
	m_ReqInsertOrder->ReqInsertOrder->Volume = index;
	m_ReqInsertOrder->ReqInsertOrder->ClientOrderID = index;
	Send(m_ReqInsertOrder);
	m_ReqInsertOrder->Deallocate();
}

void TestStepClient()
{
	WriteLog(LogLevel::Info, "TestStepClient");

	IOThread* ioThread = new IOThread("StepClient");
	StepClient stepClient;
	stepClient.SetIOThread(ioThread);
	if (!stepClient.Init())
		return;
	ioThread->Start();
	ioThread->Join();
}



