#include "TestXtpServer.h"
#include <Spark/Core/Utility/Utility.h>
#include <Spark/Core/Logger/Logger.h>
#include "TestUtility/TestUtility.h"
#include "PackageFactory.h"

using namespace std;

using namespace spark::core;
using namespace spark::network;
using namespace spark::packages;

XtpServer::XtpServer()
	:Protocol(ProtocolTypeType::Xtp, ServerTypeType::Server, g_IOModel, 0, new PackageFactory()), m_Connected(false), m_SessionID(0LL), m_RecvCount(0)
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

	m_SessionID = sessionID;
	m_Connected = true;
}
void XtpServer::OnProtocolDisConnect(SessionIDType sessionID, const char* ip, int port)
{
	WriteLog(LogLevel::Info, "XtpServer::OnDisConnect SessionID:[%lld], IP:[%s], port:[%d]", sessionID, ip, port);

	m_Connected = false;
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

	while (!xtpServer.m_Connected)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	std::this_thread::sleep_for(std::chrono::seconds(60));
	ioThread->Stop();
	ioThread->Join();
}



