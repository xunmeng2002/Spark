#pragma once
#include <string>


namespace spark::core
{
struct TimeZone
{
	std::string StartTime;
	std::string EndTime;
};

struct IPAddressField
{
	int AddressType;
	std::string IPString;
	int IP;
	int Port;
};

struct SubscribeInstrument
{
	std::string ExchangeID;
	std::string InstrumentID;
};
}
