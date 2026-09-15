#pragma once
#include <Spark/Network/NetworkExport.h>
#include <string>

namespace Spark::Network
{
void NETWORK_EXPORTS ParseAddress(const std::string& addressName, std::string& address, std::string& port);
}
