#include <Spark/Network/Protocol/StepUtility.h>
#include <Spark/Network/Protocol/Items.h>
#include <Spark/Network/Protocol/ProtocolUtility.h>
#include <Spark/Network/Protocol/ProtocolVersion.h>
#include <Spark/Core/Logger/Logger.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

using namespace std;
using namespace spark::core;

namespace spark::network
{
namespace
{
//报文头在线上出现的字段个数，HeadFromStream 要求全部到齐才算解析成功
constexpr int HeadItemCount = 6;

//对端字节不可用 std::stoi：抛出的异常会穿过线程入口，直接终止进程
bool TryParseInteger(const std::string& text, int base, long long& value)
{
	if (text.empty())
	{
		return false;
	}
	errno = 0;
	char* end = nullptr;
	long long parsed = strtoll(text.c_str(), &end, base);
	if (end == text.c_str() || *end != '\0' || errno == ERANGE)
	{
		return false;
	}
	value = parsed;
	return true;
}

//读取 buff 中 [startIndex, endIndex) 内以 SOH 结尾的十六进制字段值
bool TryReadHexField(char* buff, int startIndex, int endIndex, long long& value)
{
	int sohIndex = 0;
	if (!StepUtility::GetNextSoh(buff, startIndex, endIndex, sohIndex))
	{
		return false;
	}
	return TryParseInteger(std::string(buff + startIndex, buff + sohIndex), 16, value);
}
}

bool StepUtility::GetNextSoh(char* buff, int startIndex, int endIndex, int& sohIndex)
{
	for (int i = startIndex; i < endIndex; ++i)
	{
		if (buff[i] == SOH)
		{
			sohIndex = i;
			return true;
		}
	}
	return false;
}
bool StepUtility::GetNextEqual(char* buff, int startIndex, int endIndex, int& equalIndex)
{
	for (int i = startIndex; i < endIndex; ++i)
	{
		if (buff[i] == '=')
		{
			equalIndex = i;
			return true;
		}
	}
	return false;
}
bool StepUtility::GetNext(char* buff, int startIndex, int endIndex, unsigned short& key, std::string& value, int& sohIndex)
{
	if (!GetNextSoh(buff, startIndex, endIndex, sohIndex))
	{
		return false;
	}
	int equalIndex = 0;
	if (!GetNextEqual(buff, startIndex, sohIndex, equalIndex))
	{
		return false;
	}
	key = atoi(buff + startIndex);
	value = std::string(buff + equalIndex + 1, buff + sohIndex);
	return true;
}
bool StepUtility::GetFieldStart(char* buff, int startIndex, int endIndex, unsigned short& fieldID, int& fieldStartIndex)
{
	//i 取 '6' 的位置，标记是 "SOH + 6 + ="；i 为 0 时前面没有字节，不可能是标记
	for (int i = startIndex; i + 1 < endIndex; ++i)
	{
		if (buff[i] != '6' || buff[i + 1] != '=' || i == 0 || buff[i - 1] != SOH)
		{
			continue;
		}
		long long parsed = 0;
		if (!TryReadHexField(buff, i + 2, endIndex, parsed))
		{
			return false;
		}
		fieldStartIndex = i;
		fieldID = static_cast<unsigned short>(parsed);
		return true;
	}
	return false;
}
bool StepUtility::GetFieldEnd(char* buff, int startIndex, int endIndex, unsigned short& fieldID, int& fieldEndIndex)
{
	//i 取 '7' 的位置，标记是 "SOH + 7 + ="；i 为 0 时前面没有字节，不可能是标记
	for (int i = startIndex; i + 1 < endIndex; ++i)
	{
		if (buff[i] != '7' || buff[i + 1] != '=' || i == 0 || buff[i - 1] != SOH)
		{
			continue;
		}
		long long parsed = 0;
		if (!TryReadHexField(buff, i + 2, endIndex, parsed))
		{
			return false;
		}
		int sohIndex = 0;
		if (!GetNextSoh(buff, i, endIndex, sohIndex))
		{
			return false;
		}
		fieldID = static_cast<unsigned short>(parsed);
		fieldEndIndex = sohIndex + 1;
		return true;
	}
	return false;
}
bool StepUtility::GetNextFieldZone(char* buff, int startIndex, int endIndex, unsigned short& fieldID, int& fieldStartIndex, int& fieldEndIndex)
{
	if (!GetFieldStart(buff, startIndex, endIndex, fieldID, fieldStartIndex))
	{
		return false;
	}
	unsigned short fieldIDEnd;
	if (!GetFieldEnd(buff, fieldStartIndex, endIndex, fieldIDEnd, fieldEndIndex))
	{
		return false;
	}
	if (fieldID != fieldIDEnd)
	{
		WriteLog(LogLevel::Error, "FieldID not Match. FieldID:0x%X, FieldIDEnd:0x%X", fieldID, fieldIDEnd);
		return false;
	}
	return true;
}
const std::string& StepUtility::GetPackageStartAnchor()
{
	static const std::string anchor = []()
	{
		char buff[32] = { 0 };
		::snprintf(buff, sizeof(buff), "%c%u=%s%c", SOH, Items::Magic, ProtocolMagicText, SOH);
		return std::string(buff);
	}();
	return anchor;
}

void StepUtility::WriteString(char*& ppos, int key, bool value)
{
	int len = sprintf(ppos, "%d=%d", key, value);
	ppos += len;
	*ppos++ = SOH;
}
void StepUtility::WriteString(char*& ppos, int key, char value)
{
	int len = sprintf(ppos, "%d=%c", key, value);
	ppos += len;
	*ppos++ = SOH;
}
void StepUtility::WriteString(char*& ppos, int key, unsigned short value)
{
	int len = sprintf(ppos, "%d=%u", key, static_cast<unsigned int>(value));
	ppos += len;
	*ppos++ = SOH;
}
void StepUtility::WriteString(char*& ppos, int key, uint8_t value)
{
	int len = sprintf(ppos, "%d=%u", key, static_cast<unsigned int>(value));
	ppos += len;
	*ppos++ = SOH;
}
void StepUtility::WriteString(char*& ppos, int key, int8_t value)
{
	int len = sprintf(ppos, "%d=%d", key, static_cast<int>(value));
	ppos += len;
	*ppos++ = SOH;
}
void StepUtility::WriteString(char*& ppos, int key, int16_t value)
{
	int len = sprintf(ppos, "%d=%d", key, static_cast<int>(value));
	ppos += len;
	*ppos++ = SOH;
}
void StepUtility::WriteString(char*& ppos, int key, uint32_t value)
{
	int len = sprintf(ppos, "%d=%u", key, static_cast<unsigned int>(value));
	ppos += len;
	*ppos++ = SOH;
}
void StepUtility::WriteString(char*& ppos, int key, uint64_t value)
{
	int len = sprintf(ppos, "%d=%llu", key, static_cast<unsigned long long>(value));
	ppos += len;
	*ppos++ = SOH;
}
void StepUtility::WriteString(char*& ppos, int key, int value)
{
	int len = sprintf(ppos, "%d=%d", key, value);
	ppos += len;
	*ppos++ = SOH;
}
void StepUtility::WriteString(char*& ppos, int key, long long value)
{
	int len = sprintf(ppos, "%d=%lld", key, value);
	ppos += len;
	*ppos++ = SOH;
}
void StepUtility::WriteString(char*& ppos, int key, double value)
{
	int len = sprintf(ppos, "%d=%.6f", key, value);
	ppos += len;
	*ppos++ = SOH;
}
void StepUtility::WriteString(char*& ppos, int key, std::string value)
{
	int len = sprintf(ppos, "%d=%s", key, value.c_str());
	ppos += len;
	*ppos++ = SOH;
}
void StepUtility::WriteString(char*& ppos, int key, char* value)
{
	int len = sprintf(ppos, "%d=%s", key, value);
	ppos += len;
	*ppos++ = SOH;
}

void StepUtility::WriteHexString(char*& ppos, int key, unsigned short value)
{
	int len = sprintf(ppos, "%d=%04X", key, value);
	ppos += len;
	*ppos++ = SOH;
}

int StepUtility::HeadToStream(HeadField* head, char* buff, int size)
{
	if (size < static_cast<int>(StepMaxHeaderLen))
	{
		return 0;
	}
	int len = 0;
	len += ::snprintf(buff + len, size - len, "%c%u=%s%c", SOH, Items::Magic, ProtocolMagicText, SOH);
	len += ::snprintf(buff + len, size - len, "%u=%u%c", Items::Version, head->Version, SOH);
	len += ::snprintf(buff + len, size - len, "%u=%04X%c", Items::PackageID, head->PackageID, SOH);
	len += ::snprintf(buff + len, size - len, "%u=%05u%c", Items::BodyLen, head->BodyLen, SOH);
	len += ::snprintf(buff + len, size - len, "%u=%13d%c", Items::MsgSeqNum, head->MsgSeqNum, SOH);
	len += ::snprintf(buff + len, size - len, "%u=%d", Items::MessageChain, head->MessageChain);
	//最后一个不能使用sprintf赋值，因为sprintf会在末尾自动补上0
	buff[len] = SOH;
	return len + 1;
}
bool StepUtility::HeadFromStream(char* buff, int startIndex, int endIndex, HeadField* head, int& headEndIndex)
{
	//跳过报文首个SOH符号
	startIndex += 1;
	int seenCount = 0;
	unsigned short key;
	std::string value;
	int sohIndex;
	while (startIndex < endIndex)
	{
		if (!GetNext(buff, startIndex, endIndex, key, value, sohIndex))
		{
			break;
		}
		long long parsed = 0;
		switch (key)
		{
		case Items::Magic:
			if (value != ProtocolMagicText)
			{
				return false;
			}
			head->Magic = ProtocolMagicValue;
			break;
		case Items::Version:
			if (!TryParseInteger(value, 10, parsed))
			{
				return false;
			}
			head->Version = static_cast<UInt16Type>(parsed);
			break;
		case Items::PackageID:
			if (!TryParseInteger(value, 16, parsed))
			{
				return false;
			}
			head->PackageID = static_cast<UInt16Type>(parsed);
			break;
		case Items::BodyLen:
			if (!TryParseInteger(value, 10, parsed) || parsed < 0 || parsed > 0xFFFF)
			{
				return false;
			}
			head->BodyLen = static_cast<UInt16Type>(parsed);
			break;
		case Items::MsgSeqNum:
			if (!TryParseInteger(value, 10, parsed))
			{
				return false;
			}
			head->MsgSeqNum = static_cast<Int32Type>(parsed);
			break;
		case Items::MessageChain:
			if (!TryParseInteger(value, 10, parsed))
			{
				return false;
			}
			head->MessageChain = (parsed != 0);
			break;
		default:
			//不是包头字段，说明包头到此结束，startIndex 就是包体起始位置
			headEndIndex = startIndex;
			return seenCount == HeadItemCount;
		}
		++seenCount;
		startIndex = sohIndex + 1;
	}
	//缓冲正好止于包头末尾（包体长度为 0 且报尾尚未到达）。六个字段都校验通过才算解析成功，
	//此时 startIndex 就停在最后一个字段之后，即包体起点
	headEndIndex = startIndex;
	return seenCount == HeadItemCount;
}
int StepUtility::TailToStream(TailField* tail, char* buff, int size)
{
	if (size < static_cast<int>(StepTailLen))
	{
		return 0;
	}
	int len = ::snprintf(buff, static_cast<size_t>(size), "%u=%08X", Items::CheckSum, static_cast<unsigned int>(tail->CheckSum));
	if (len != static_cast<int>(StepTailLen) - 1)
	{
		return 0;
	}
	//最后一个不能使用sprintf赋值，因为sprintf会在末尾自动补上0；
	//snprintf 写满 8 位十六进制要 11 字节（含结尾的 0），而线上报尾只有 11 字节，所以 SOH 手工补
	buff[len] = static_cast<char>(SOH);
	return len + 1;
}
bool StepUtility::TailFromStream(char* buff, int startIndex, int endIndex, TailField* tail)
{
	bool parsed = false;
	while (startIndex < endIndex)
	{
		unsigned short key;
		string value;
		int sohIndex;
		if (!GetNext(buff, startIndex, endIndex, key, value, sohIndex))
		{
			break;
		}
		if (key != Items::CheckSum)
		{
			WriteLog(LogLevel::Warning, "UnExpected Key:0x%X for TailField.", key);
			return false;
		}
		long long checksum = 0;
		if (!TryParseInteger(value, 16, checksum) || checksum < 0 || checksum > 0xFFFFFFFF)
		{
			return false;
		}
		tail->CheckSum = static_cast<Int32Type>(static_cast<unsigned int>(checksum));
		parsed = true;
		startIndex = sohIndex + 1;
	}
	return parsed;
}
}
