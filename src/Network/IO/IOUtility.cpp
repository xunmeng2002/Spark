#include <Spark/Network/IO/IOUtility.h>


namespace spark::network
{
void ParseAddress(const std::string& addressName, std::string& address, std::string& port)
{
	auto index = addressName.find(':', 0);
	address = addressName.substr(0, index);
	port = addressName.substr(index + 1);
}
}

