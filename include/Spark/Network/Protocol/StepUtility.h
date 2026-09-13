#pragma once
#include <Spark/Network/NetworkExport.h>
#include <Spark/Network/Protocol/Head.h>
#include <string>
#include <unordered_map>


//报尾固定为 "5=" + CRC32C 的 8 位十六进制 + SOH
constexpr unsigned int StepTailLen = 2u + 8u + 1u;
//包头由 key=value 字段串成、长度不固定，这个值只是"超过它还没解析出包头就判定为非法"的上界
constexpr unsigned int StepMaxHeaderLen = 128u;
constexpr unsigned int SOH = 1u;

namespace spark::network
{
class NETWORK_EXPORTS StepUtility
{
public:
	static bool GetNextSoh(char* buff, int startIndex, int endIndex, int& sohIndex);
	static bool GetNextEqual(char* buff, int startIndex, int endIndex, int& equalIndex);
	static bool GetNext(char* buff, int startIndex, int endIndex, unsigned short& key, std::string& value, int& sohIndex);
	static bool GetFieldStart(char* buff, int startIndex, int endIndex, unsigned short& fieldID, int& fieldStartIndex);
	static bool GetFieldEnd(char* buff, int startIndex, int endIndex, unsigned short& fieldID, int& fieldEndIndex);
	static bool GetNextFieldZone(char* buff, int startIndex, int endIndex, unsigned short& fieldID, int& fieldStartIndex, int& fieldEndIndex);
	static bool GetPackageStart(char* buff, int startIndex, int endIndex, int& packageStartIndex);
	//报文起始锚点，形如 SOH + "0=SPK2" + SOH，与 ProtocolVersion.h 的魔术字是同一串字节
	static const std::string& GetPackageStartAnchor();

	static void WriteString(char*& ppos, int key, bool value);
	static void WriteString(char*& ppos, int key, char value);
	static void WriteString(char*& ppos, int key, unsigned short value);
	static void WriteString(char*& ppos, int key, int value);
	static void WriteString(char*& ppos, int key, long long value);
	static void WriteString(char*& ppos, int key, double value);
	static void WriteString(char*& ppos, int key, std::string value);
	static void WriteString(char*& ppos, int key, char* value);
	template<typename T>
	static void WriteString(char*& ppos, int key, T value)
	{
		int len = sprintf(ppos, "%d=%s", key, value);
		ppos += len;
		*ppos++ = SOH;
	}
	static void WriteHexString(char*& ppos, int key, unsigned short value);


	//返回实际写入的包头长度
	static int HeadToStream(HeadField* head, char* buff, int size);
	//成功时把包头结束位置写入 headEndIndex，即包体的起始位置
	static bool HeadFromStream(char* buff, int startIndex, int endIndex, HeadField* head, int& headEndIndex);
	//返回实际写入的报尾长度
	static int TailToStream(TailField* tail, char* buff, int size);
	static bool TailFromStream(char* buff, int startIndex, int endIndex, TailField* tail);

};
}
