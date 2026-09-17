#pragma once
#include <Spark/Network/NetworkExport.h>
#include <Spark/Network/Io/IoBase.h>


namespace Spark::Network
{
class NETWORK_EXPORTS IoFactory
{
public:
    static IoBase* CreateIo(ServerTypeType serverType, const char* address,  IoModelType ioModel = IoModelType::Select, int milliSeconds = 100);
};
}


