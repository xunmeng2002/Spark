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
    :Protocol(ProtocolTypeType::Xtp, ServerTypeType::Server, IoModel, 0, new PackageFactory()), connected_(false), sessionId_(0LL), recvCount_(0)
{
    Subscribe(this);
    RegisterFront(TcpAddress);
}
XtpServer::~XtpServer()
{
}

void XtpServer::OnProtocolConnect(SessionIdType sessionId, const char* ip, int port)
{
    WriteLog(LogLevel::Info, "XtpServer::OnConnect SessionId:[%lld], IP:[%s], port:[%d]", sessionId, ip, port);

    sessionId_ = sessionId;
    connected_ = true;
}
void XtpServer::OnProtocolDisConnect(SessionIdType sessionId, const char* ip, int port)
{
    WriteLog(LogLevel::Info, "XtpServer::OnDisConnect SessionId:[%lld], IP:[%s], port:[%d]", sessionId, ip, port);

    connected_ = false;
}
void XtpServer::OnMessage(Package* xtpPackage)
{
    ++recvCount_;
    //if ((recvCount_) % 1000 == 0)
    {
        WriteLog(LogLevel::Info, "OnMessage SessionId:[%lld], %s", xtpPackage->SessionId, xtpPackage->GetDebugString());
    }

    Send(xtpPackage);
}


void TestXtpServer()
{
    WriteLog(LogLevel::Info, "TestXtpServer");

    IoThread* ioThread = new IoThread("XtpServer");
    XtpServer xtpServer;
    xtpServer.SetIoThread(ioThread);
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



