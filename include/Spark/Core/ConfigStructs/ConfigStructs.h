#pragma once
#include <string>

namespace Spark::Core
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
    std::string ExchangeId;
    std::string InstrumentId;
};
}
