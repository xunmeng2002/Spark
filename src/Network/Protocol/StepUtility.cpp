#include <Spark/Network/Protocol/StepUtility.h>
#include <Spark/Network/Protocol/Items.h>
#include <Spark/Core/Logger/Logger.h>

using namespace std;
using namespace spark::core;

namespace spark::network
{
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
	for (int i = startIndex; i < endIndex; ++i)
	{
		if (buff[i - 1] == SOH && buff[i] == '6' && buff[i + 1] == '=')
		{
			fieldStartIndex = i;
			fieldID = stoi(buff + i + 2, nullptr, 16);
			return true;
		}
	}
	return false;
}
bool StepUtility::GetFieldEnd(char* buff, int startIndex, int endIndex, unsigned short& fieldID, int& fieldEndIndex)
{
	for (int i = startIndex; i < endIndex; ++i)
	{
		if (buff[i - 1] == SOH && buff[i] == '7' && buff[i + 1] == '=')
		{
			fieldID = stoi(buff + i + 2, nullptr, 16);
			int sohIndex = 0;
			if (!GetNextSoh(buff, i, endIndex, sohIndex))
			{
				return false;
			}
			fieldEndIndex = sohIndex + 1;
			return true;
		}
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
	if (!GetFieldEnd(buff, startIndex, endIndex, fieldIDEnd, fieldEndIndex))
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
bool StepUtility::GetPackageStart(char* buff, int startIndex, int endIndex, int& packageStartIndex)
{
	for (int i = startIndex; i < endIndex; ++i)
	{
		if (buff[i] == SOH && buff[i + 1] == '1' && buff[i + 2] == '=')
		{
			packageStartIndex = i;
			return true;
		}
	}
	return false;
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
	int len = sprintf(ppos, "%d=%d", key, value);
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

void StepUtility::HeadToStream(HeadField* head, char* buff, int size)
{
	int len = 0;
	len += ::sprintf(buff + len, "%c%u=%04X%c", SOH, Items::PackageID, head->PackageID, SOH);
	len += ::sprintf(buff + len, "%u=%05u%c", Items::BodyLen, head->BodyLen, SOH);
	len += ::sprintf(buff + len, "%u=%13d%c", Items::MsgSeqNum, head->MsgSeqNum, SOH);
	len += ::sprintf(buff + len, "%u=%d", Items::MessageChain, head->MessageChain);
	//最后一个不能使用sprintf赋值，因为sprintf会在末尾自动补上0
	buff[len] = SOH;
}
bool StepUtility::HeadFromStream(char* buff, int startIndex, int endIndex, HeadField* head)
{
	//跳过报文首个SOH符号
	startIndex += 1;
	unsigned short key;
	std::string value;
	int sohIndex;
	while (startIndex < endIndex)
	{
		if (GetNext(buff, startIndex,  endIndex, key, value, sohIndex))
		{
			switch (key)
			{
			case Items::PackageID:
				head->PackageID = stoi(value.c_str(), nullptr, 16);
				break;
			case Items::BodyLen:
				head->BodyLen = atoi(value.c_str());
				break;
			case Items::MsgSeqNum:
				head->MsgSeqNum = atoi(value.c_str());
				break;
			case Items::MessageChain:
				head->MessageChain = atoi(value.c_str());
				break;
			default:
				WriteLog(LogLevel::Warning, "UnExpected Key:0x%X For HeadField, buff:[%s]", key, buff);
				return false;
			}
			startIndex = sohIndex + 1;
		}
		else
		{
			break;
		}
	}
	return true;
}
void StepUtility::TailToStream(TailField* tail, char* buff, int size)
{
	sprintf(buff, "%u=%05u%c", Items::CheckSum, tail->CheckSum, SOH);
}
bool StepUtility::TailFromStream(char* buff, int startIndex, int endIndex, TailField* tail)
{
	while (startIndex < endIndex)
	{
		unsigned short key;
		string value;
		int sohIndex;
		if (GetNext(buff, startIndex, endIndex, key, value, sohIndex))
		{
			switch (key)
			{
			case Items::CheckSum:
				tail->CheckSum = atoi(value.c_str());
				break;
			default:
				WriteLog(LogLevel::Warning, "UnExpected Key:0x%X for TailField.", key);
				return false;
			}
			startIndex = sohIndex + 1;
		}
		else
		{
			break;
		}
	}
	return true;
}
}

