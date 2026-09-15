#include "Tcp/TcpSelect/TcpSelectServer.h"
#include <Spark/Core/Logger/Logger.h>
#include "Tcp/TcpUtility.h"
#include <string.h>


namespace Spark::Network
{
TcpSelectServer::TcpSelectServer(const char* addressName, int milliSeconds)
	:TcpSelectBase(ServerTypeType::Server, addressName, milliSeconds)
{
}
}
