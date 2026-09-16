#pragma once
#include <Spark/Network/NetworkExport.h>
#include <Spark/Network/IO/IOBase.h>


namespace Spark::Network
{
class NETWORK_EXPORTS IOFactory
{
public:
	static IOBase* CreateIo(ServerTypeType serverType, const char* address,  IoModelType ioModel = IoModelType::Select, int milliSeconds = 100);
};
}


