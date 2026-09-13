#pragma once
#include <Spark/Network/NetworkExport.h>


namespace spark::network
{
short NETWORK_EXPORTS CalculateSum(unsigned char* buff, int len);
unsigned int NETWORK_EXPORTS CalculateCrc32c(const unsigned char* buff, int len);
//在 dataLength 字节里逐字节找 patternLength 字节的模式，找到返回 true 并把下标写入 offset
bool NETWORK_EXPORTS FindBytes(const char* data, unsigned int dataLength, const char* pattern, unsigned int patternLength, unsigned int& offset);
}
