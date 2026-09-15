#pragma once
#include <Spark/Network/NetworkExport.h>
#include <Spark/Network/IO/IOBase.h>


namespace Spark::Network
{
class NETWORK_EXPORTS IOFactory
{
public:
	static IOBase* CreateIO(ServerTypeType serverType, const char* address,  IOModelType ioModel = IOModelType::Select, int milliSeconds = 100);
};
}


