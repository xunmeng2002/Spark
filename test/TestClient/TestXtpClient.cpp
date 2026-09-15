#include "TestXtpClient.h"
#include "TestUtility/TestUtility.h"
#include "PackageFactory.h"
#include "Packages.h"
#include <Spark/Core/Utility/Utility.h>
#include <Spark/Core/Logger/Logger.h>
#include <Spark/Core/Utility/TimeUtility.h>
#include <Spark/Network/IO/IOThread.h>
#include <Spark/TemplateLib/ObjectPool/ObjectPool.h>

using namespace std;

using namespace Spark;
using namespace Spark::Core;
using namespace Spark::Network;
using namespace Spark::Packages;

XtpClient::XtpClient()
	:Protocol(ProtocolTypeType::Xtp, ServerTypeType::Client, g_IOModel, 0, new PackageFactory()), connected_(false), sessionId_(0LL), m_RecvCount(0)
{
	m_ReqInsertOrder = new ReqInsertOrderPackage();
	Subscribe(this);
	RegisterFront(g_Address);
}
XtpClient::~XtpClient()
{
}
void XtpClient::OnProtocolConnect(SessionIDType sessionID, const char* ip, int port)
{
	WriteLog(LogLevel::Info, "XtpClient::OnConnect SessionID:[%lld], IP:[%s], port:[%d]", sessionID, ip, port);

	sessionId_ = sessionID;
	connected_ = true;
	m_StartTime = chrono::steady_clock::now();
	SendReqInsertOrder(++m_RecvCount);
}
void XtpClient::OnProtocolDisConnect(SessionIDType sessionID, const char* ip, int port)
{
	WriteLog(LogLevel::Info, "XtpClient::OnDisConnect SessionID:[%lld], IP:[%s], port:[%d]", sessionID, ip, port);

	connected_ = false;
	m_RecvCount = 0;
}
void XtpClient::OnMessage(Package* package)
{
	++m_RecvCount;
	//if (m_RecvCount % 10000 == 0)
	{
		WriteLog(LogLevel::Info, "OnMessage: %s", package->GetDebugString());
	}
	if (m_RecvCount < 10)
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

void XtpClient::SendReqInsertOrder(int index)
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


void TestXtpClient()
{
	WriteLog(LogLevel::Info, "TestXtpClient");

	IOThread* ioThread = new IOThread("XtpClient");
	XtpClient xtpClient;
	xtpClient.SetIOThread(ioThread);
	if (!xtpClient.Init())
		return;
	ioThread->Start();
	ioThread->Join();
}

