#include <Spark/Network/Protocol/ProtocolUtility.h>
#include <array>
#include <cstring>


namespace spark::network
{
namespace
{
//CRC-32C（Castagnoli）的反射形式多项式，初值与末异或都是 0xFFFFFFFF
constexpr unsigned int Crc32cReflectedPolynomial = 0x82F63B78u;
constexpr unsigned int Crc32cInitValue = 0xFFFFFFFFu;

constexpr std::array<unsigned int, 256> MakeCrc32cTable()
{
    std::array<unsigned int, 256> table{};
    for (auto i = 0u; i < 256; i++)
    {
        unsigned int remainder = i;
        for (int bit = 0; bit < 8; ++bit)
        {
            remainder = (remainder & 1u) ? ((remainder >> 1) ^ Crc32cReflectedPolynomial) : (remainder >> 1);
        }
        table.at(i) = remainder;
    }
	return table;
}

constexpr std::array<unsigned int, 256> Crc32cTable = MakeCrc32cTable();
}

unsigned int CalculateCrc32c(const unsigned char* buff, int len)
{
	unsigned int remainder = Crc32cInitValue;
	for (int i = 0; i < len; ++i)
	{
		remainder = Crc32cTable[(remainder ^ buff[i]) & 0xFFu] ^ (remainder >> 8);
	}
	return remainder ^ Crc32cInitValue;
}

bool FindBytes(const char* data, unsigned int dataLength, const char* pattern, unsigned int patternLength, unsigned int& offset)
{
	if (data == nullptr || pattern == nullptr || patternLength == 0 || dataLength < patternLength)
	{
		return false;
	}
	for (unsigned int i = 0; i + patternLength <= dataLength; ++i)
	{
		if (::memcmp(data + i, pattern, patternLength) == 0)
		{
			offset = i;
			return true;
		}
	}
	return false;
}
}
