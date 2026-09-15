#include "TestXtpServer.h"
#include <Spark/Core/Utility/Utility.h>
#include <Spark/Core/Logger/Logger.h>
#include "TestUtility/TestUtility.h"
#include "PackageFactory.h"

using namespace std;

using namespace Spark::Core;
using namespace Spark::Network;
using namespace Spark::Packages;

XtpServer::XtpServer()
	:Protocol(ProtocolTypeType::Xtp, ServerTypeType::Server, g_IOModel, 0, new PackageFactory()), connected_(false), sessionId_(0LL), m_RecvCount(0)
{
	Subscribe(this);
	RegisterFront(g_Address);
}
XtpServer::~XtpServer()
{
}

void XtpServer::OnProtocolConnect(SessionIDType sessionID, const char* ip, int port)
{
	WriteLog(LogLevel::Info, "XtpServer::OnConnect SessionID:[%lld], IP:[%s], port:[%d]", sessionID, ip, port);

	sessionId_ = sessionID;
	connected_ = true;
}
void XtpServer::OnProtocolDisConnect(SessionIDType sessionID, const char* ip, int port)
{
	WriteLog(LogLevel::Info, "XtpServer::OnDisConnect SessionID:[%lld], IP:[%s], port:[%d]", sessionID, ip, port);

	connected_ = false;
}
void XtpServer::OnMessage(Package* xtpPackage)
{
	++m_RecvCount;
	//if ((m_RecvCount) % 1000 == 0)
	{
		WriteLog(LogLevel::Info, "OnMessage SessionID:[%lld], %s", xtpPackage->SessionID, xtpPackage->GetDebugString());
	}

	Send(xtpPackage);
}


void TestXtpServer()
{
	WriteLog(LogLevel::Info, "TestXtpServer");

	IOThread* ioThread = new IOThread("XtpServer");
	XtpServer xtpServer;
	xtpServer.SetIOThread(ioThread);
	if (!xtpServer.Init())
		return;
	ioThread->Start();

	while (!xtpServer.connected_)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	std::this_thread::sleep_for(std::chrono::seconds(60));
	ioThread->Stop();
	ioThread->Join();
}



