#include "TestStepServer.h"
#include <Spark/Core/Utility/Utility.h>
#include <Spark/Core/Logger/Logger.h>
#include "TestUtility/TestUtility.h"
#include <Spark/Network/Protocol/PackageFactory.h>

using namespace std;


StepServer::StepServer()
	:Protocol(ProtocolTypeType::Step, ServerTypeType::Server, g_IOModel, 0, new PackageFactory()), m_Connected(false), m_SessionID(0LL), m_RecvCount(0)
{
	Subscribe(this);
	RegisterFront(g_Address);
}
StepServer::~StepServer()
{
}

void StepServer::OnProtocolConnect(SessionIDType sessionID, const char* ip, int port)
{
	WriteLog(LogLevel::Info, "StepServer::OnConnect SessionID:[%lld], IP:[%s], port:[%d]", sessionID, ip, port);

	m_SessionID = sessionID;
	m_Connected = true;
}
void StepServer::OnProtocolDisConnect(SessionIDType sessionID, const char* ip, int port)
{
	WriteLog(LogLevel::Info, "StepServer::OnDisConnect SessionID:[%lld], IP:[%s], port:[%d]", sessionID, ip, port);

	m_Connected = false;
}
void StepServer::OnMessage(Package* stepPackage)
{
	if ((++m_RecvCount) % 1000 == 0)
	{
		WriteLog(LogLevel::Info, "OnMessage SessionID:[%lld], %s", stepPackage->SessionID, stepPackage->GetDebugString());
	}

	Send(stepPackage);
}


void TestStepServer()
{
	WriteLog(LogLevel::Info, "TestStepServer");

	IOThread* ioThread = new IOThread("StepServer");
	StepServer StepServer;
	StepServer.SetIOThread(ioThread);
	if (!StepServer.Init())
		return;
	ioThread->Start();

	while (!StepServer.m_Connected)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	std::this_thread::sleep_for(std::chrono::seconds(90));
	ioThread->Stop();
	ioThread->Join();
}



