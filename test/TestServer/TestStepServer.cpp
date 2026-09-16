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
	:Protocol(ProtocolTypeType::Step, ServerTypeType::Server, IoModel, 0, new PackageFactory()), connected_(false), sessionId_(0LL), recvCount_(0)
{
	Subscribe(this);
	RegisterFront(TcpAddress);
}
StepServer::~StepServer()
{
}

void StepServer::OnProtocolConnect(SessionIdType sessionId, const char* ip, int port)
{
	WriteLog(LogLevel::Info, "StepServer::OnConnect SessionId:[%lld], IP:[%s], port:[%d]", sessionId, ip, port);

	sessionId_ = sessionId;
	connected_ = true;
}
void StepServer::OnProtocolDisConnect(SessionIdType sessionId, const char* ip, int port)
{
	WriteLog(LogLevel::Info, "StepServer::OnDisConnect SessionId:[%lld], IP:[%s], port:[%d]", sessionId, ip, port);

	connected_ = false;
}
void StepServer::OnMessage(Package* stepPackage)
{
	if ((++recvCount_) % 1000 == 0)
	{
		WriteLog(LogLevel::Info, "OnMessage SessionId:[%lld], %s", stepPackage->SessionId, stepPackage->GetDebugString());
	}

	Send(stepPackage);
}


void TestStepServer()
{
	WriteLog(LogLevel::Info, "TestStepServer");

	IOThread* ioThread = new IOThread("StepServer");
	StepServer StepServer;
	StepServer.SetIoThread(ioThread);
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



