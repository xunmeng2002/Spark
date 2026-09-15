#include "TestStepServer.h"
#include <Spark/Core/Utility/Utility.h>
#include <Spark/Core/Logger/Logger.h>
#include "TestUtility/TestUtility.h"
#include "PackageFactory.h"

using namespace std;
using namespace Spark::Core;
using namespace Spark::Network;
using namespace Spark::Packages;

StepServer::StepServer()
	:Protocol(ProtocolTypeType::Step, ServerTypeType::Server, g_IOModel, 0, new PackageFactory()), connected_(false), sessionId_(0LL), m_RecvCount(0)
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

	sessionId_ = sessionID;
	connected_ = true;
}
void StepServer::OnProtocolDisConnect(SessionIDType sessionID, const char* ip, int port)
{
	WriteLog(LogLevel::Info, "StepServer::OnDisConnect SessionID:[%lld], IP:[%s], port:[%d]", sessionID, ip, port);

	connected_ = false;
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

	while (!StepServer.connected_)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	std::this_thread::sleep_for(std::chrono::seconds(90));
	ioThread->Stop();
	ioThread->Join();
}



