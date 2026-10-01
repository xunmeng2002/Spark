#pragma once
#include <Spark/Network/NetworkExport.h>

#include <cstddef>

namespace Spark::Network
{
unsigned int NETWORK_EXPORTS CalculateCrc32c(const unsigned char* buff, size_t len);
//在 dataLength 字节里逐字节找 patternLength 字节的模式，找到返回 true 并把下标写入 offset
bool NETWORK_EXPORTS FindBytes(const char* data, size_t dataLength, const char* pattern, size_t patternLength, size_t& offset);
}
