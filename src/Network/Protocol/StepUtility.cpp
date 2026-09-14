#include <Spark/Network/Protocol/StepUtility.h>
#include <Spark/Network/Protocol/Items.h>
#include <Spark/Network/Protocol/ProtocolVersion.h>
#include <Spark/Core/Logger/Logger.h>
#include <stdio.h>

using namespace std;
using namespace spark::core;

namespace spark::network
{
namespace
{
//报文头在线上出现的字段个数，HeadFromStream 要求全部到齐才算解析成功
constexpr int HeadItemCount = 6;

//读取 buff 中 [startIndex, endIndex) 内以 SOH 结尾的十六进制字段值。
//from_chars 对 UInt16Type 自带越界拒绝，无需再手写上界
bool TryReadHexField(char* buff, int startIndex, int endIndex, UInt16Type& value)
{
	int sohIndex = 0;
	if (!StepUtility::GetNextSoh(buff, startIndex, endIndex, sohIndex))
	{
		return false;
	}
	return StepUtility::ParseInteger(std::string(buff + startIndex, buff + sohIndex), value, 16);
}

//包头首字段同时也是帧起始锚点，必须是同一次格式化，抽出来防止两处各自漂移
bool AppendPackageMagicField(StepWriteCursor& cursor)
{
	return cursor.AppendField("{:c}{:04X}={:s}", SOH, Items::Magic, ProtocolMagicText);
}

//判断 buff 的 index 处是不是 expectedKey 后跟 '=' 的完整键
bool IsKeyAt(char* buff, int index, int endIndex, UInt16Type expectedKey)
{
	if (index + StepKeyTextLen >= endIndex || buff[index + StepKeyTextLen] != '=')
	{
		return false;
	}
	UInt16Type parsedKey = 0;
	if (!StepUtility::ParseInteger(std::string(buff + index, buff + index + StepKeyTextLen), parsedKey, 16))
	{
		return false;
	}
	return parsedKey == expectedKey;
}

//扫描标记键（0006 / 0007），回填其后的十六进制 fieldID、标记起始位置、以及标记字段之后的下一个位置。
//GetFieldStart 取前者、GetFieldEnd 取后者，两个入口除此以外完全一致
bool LocateFieldMarker(char* buff, int startIndex, int endIndex, UInt16Type markerKey,
	UInt16Type& fieldID, int& markerStartIndex, int& afterMarkerIndex)
{
	//标记形如 "SOH + '0006' + '='"；i 指向键的首字节，i 为 0 时前面没有字节，不可能是标记
	for (int i = startIndex; i < endIndex; ++i)
	{
		if (i == 0 || buff[i - 1] != SOH || !IsKeyAt(buff, i, endIndex, markerKey))
		{
			continue;
		}
		if (!TryReadHexField(buff, i + StepKeyTextLen + 1, endIndex, fieldID))
		{
			return false;
		}
		int sohIndex = 0;
		if (!StepUtility::GetNextSoh(buff, i, endIndex, sohIndex))
		{
			return false;
		}
		markerStartIndex = i;
		afterMarkerIndex = sohIndex + 1;
		return true;
	}
	return false;
}
}

StepWriteCursor::StepWriteCursor(char* buffer, int capacity)
	: buffer_begin_(buffer), capacity_(capacity), written_length_(0), is_truncated_(false)
{
}
int StepWriteCursor::GetWrittenLength() const
{
	return written_length_;
}
bool StepWriteCursor::IsTruncated() const
{
	return is_truncated_;
}
char* StepWriteCursor::GetWritePosition()
{
    return buffer_begin_ + written_length_;
}
int StepWriteCursor::GetRemainingLength() const
{
    return capacity_ - written_length_;
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
bool StepUtility::GetNext(char* buff, int startIndex, int endIndex, UInt16Type& key, std::string& value, int& sohIndex)
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
	UInt16Type parsedKey = 0;
	if (!ParseInteger(std::string(buff + startIndex, buff + equalIndex), parsedKey, 16))
	{
		return false;
	}
	key = parsedKey;
	value = std::string(buff + equalIndex + 1, buff + sohIndex);
	return true;
}
bool StepUtility::GetFieldStart(char* buff, int startIndex, int endIndex, UInt16Type& fieldID, int& fieldStartIndex)
{
	int afterMarkerIndex = 0;
	return LocateFieldMarker(buff, startIndex, endIndex, Items::FieldStart, fieldID, fieldStartIndex, afterMarkerIndex);
}
bool StepUtility::GetFieldEnd(char* buff, int startIndex, int endIndex, UInt16Type& fieldID, int& fieldEndIndex)
{
	int markerStartIndex = 0;
	return LocateFieldMarker(buff, startIndex, endIndex, Items::FieldEnd, fieldID, markerStartIndex, fieldEndIndex);
}
bool StepUtility::GetNextFieldZone(char* buff, int startIndex, int endIndex, UInt16Type& fieldID, int& fieldStartIndex, int& fieldEndIndex)
{
	if (!GetFieldStart(buff, startIndex, endIndex, fieldID, fieldStartIndex))
	{
		return false;
	}
	UInt16Type fieldIDEnd;
	if (!GetFieldEnd(buff, fieldStartIndex, endIndex, fieldIDEnd, fieldEndIndex))
	{
		return false;
	}
	if (fieldID != fieldIDEnd)
	{
		WriteLog(LogLevel::Error, "FieldID not Match. FieldID:0x%X, FieldIDEnd:0x%X", static_cast<unsigned int>(fieldID), static_cast<unsigned int>(fieldIDEnd));
		return false;
	}
	return true;
}
const std::string& StepUtility::GetPackageStartAnchor()
{
	static const std::string anchor = []()
	{
		char buff[StepHeadLen] = { 0 };
		StepWriteCursor cursor(buff, static_cast<int>(sizeof(buff)));
		AppendPackageMagicField(cursor);
		return std::string(buff, static_cast<std::size_t>(cursor.GetWrittenLength()));
	}();
	return anchor;
}

void StepUtility::WriteString(StepWriteCursor& cursor, UInt16Type key, BoolType value)
{
	cursor.AppendField("{:04X}={:d}", key, value);
}
void StepUtility::WriteString(StepWriteCursor& cursor, UInt16Type key, char value)
{
	cursor.AppendField("{:04X}={:c}", key, value);
}
void StepUtility::WriteString(StepWriteCursor& cursor, UInt16Type key, Int8Type value)
{
	cursor.AppendField("{:04X}={:d}", key, value);
}
void StepUtility::WriteString(StepWriteCursor& cursor, UInt16Type key, UInt8Type value)
{
	cursor.AppendField("{:04X}={:d}", key, value);
}
void StepUtility::WriteString(StepWriteCursor& cursor, UInt16Type key, Int16Type value)
{
	cursor.AppendField("{:04X}={:d}", key, value);
}
void StepUtility::WriteString(StepWriteCursor& cursor, UInt16Type key, UInt16Type value)
{
	cursor.AppendField("{:04X}={:d}", key, value);
}
void StepUtility::WriteString(StepWriteCursor& cursor, UInt16Type key, Int32Type value)
{
	cursor.AppendField("{:04X}={:d}", key, value);
}
void StepUtility::WriteString(StepWriteCursor& cursor, UInt16Type key, UInt32Type value)
{
	cursor.AppendField("{:04X}={:d}", key, value);
}
void StepUtility::WriteString(StepWriteCursor& cursor, UInt16Type key, Int64Type value)
{
	cursor.AppendField("{:04X}={:d}", key, value);
}
void StepUtility::WriteString(StepWriteCursor& cursor, UInt16Type key, UInt64Type value)
{
	cursor.AppendField("{:04X}={:d}", key, value);
}
void StepUtility::WriteString(StepWriteCursor& cursor, UInt16Type key, DoubleType value)
{
	cursor.AppendField("{:04X}={:.6f}", key, value);
}
void StepUtility::WriteString(StepWriteCursor& cursor, UInt16Type key, std::string value)
{
	cursor.AppendField("{:04X}={:s}", key, value);
}

void StepUtility::WriteHexString(StepWriteCursor& cursor, UInt16Type key, UInt16Type value)
{
	cursor.AppendField("{:04X}={:04X}", key, value);
}

int StepUtility::HeadToStream(HeadField* head, char* buff, int size)
{
	if (size < static_cast<int>(StepHeadLen))
	{
		return 0;
	}
	StepWriteCursor cursor(buff, size);
	if (!AppendPackageMagicField(cursor)
		|| !cursor.AppendField("{:04X}={:04X}", Items::Version, head->Version)
		|| !cursor.AppendField("{:04X}={:04X}", Items::PackageID, head->PackageID)
		|| !cursor.AppendField("{:04X}={:04X}", Items::BodyLen, head->BodyLen)
		|| !cursor.AppendField("{:04X}={:08X}", Items::MsgSeqNum, head->MsgSeqNum)
		|| !cursor.AppendField("{:04X}={:d}", Items::MessageChain, head->MessageChain))
	{
		return 0;
	}
	//包头每一位都是定宽字段，总长与取值无关。写出的长度不是 StepHeadLen 说明格式改漏了，
	//这里返回 0 让调用方当成失败，而不是发一个长度已经变了的帧出去
	if (cursor.GetWrittenLength() != static_cast<int>(StepHeadLen))
	{
		return 0;
	}
	return cursor.GetWrittenLength();
}
bool StepUtility::HeadFromStream(char* buff, int startIndex, int endIndex, HeadField* head, int& headEndIndex)
{
	//跳过报文首个SOH符号
	startIndex += 1;
	int seenCount = 0;
	UInt16Type key;
	std::string value;
	int sohIndex;
	while (startIndex < endIndex)
	{
		if (!GetNext(buff, startIndex, endIndex, key, value, sohIndex))
		{
			break;
		}
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
			if (!ParseInteger(value, head->Version, 16))
			{
				return false;
			}
			break;
		case Items::PackageID:
			if (!ParseInteger(value, head->PackageID, 16))
			{
				return false;
			}
			break;
		case Items::BodyLen:
			if (!ParseInteger(value, head->BodyLen, 16))
			{
				return false;
			}
			break;
		case Items::MsgSeqNum:
			if (!ParseInteger(value, head->MsgSeqNum, 16))
			{
				return false;
			}
			break;
		case Items::MessageChain:
		{
			//MessageChain 只写 1 位十进制数字，两种进制下逐字节相同，按 10 解析即可
			UInt16Type messageChain = 0;
			if (!ParseInteger(value, messageChain, 10))
			{
				return false;
			}
			head->MessageChain = (messageChain != 0);
			break;
		}
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
	int len = ::snprintf(buff, static_cast<size_t>(size), "%04X=%08X", static_cast<unsigned int>(Items::CheckSum), tail->CheckSum);
	if (len != static_cast<int>(StepTailLen) - 1)
	{
		return 0;
	}
	//最后一个不能使用sprintf赋值，因为sprintf会在末尾自动补上0；
	//snprintf 写满定长报尾要 StepTailLen 字节（含结尾的 0），而线上报尾没有那个 0，所以 SOH 手工补
	buff[len] = static_cast<char>(SOH);
	return len + 1;
}
bool StepUtility::TailFromStream(char* buff, int startIndex, int endIndex, TailField* tail)
{
	bool parsed = false;
	while (startIndex < endIndex)
	{
		UInt16Type key;
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
		if (!ParseInteger(value, tail->CheckSum, 16))
		{
			return false;
		}
		parsed = true;
		startIndex = sohIndex + 1;
	}
	return parsed;
}
}
