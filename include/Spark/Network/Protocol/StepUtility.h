#pragma once
#include <Spark/Network/NetworkExport.h>
#include <Spark/Network/Protocol/Head.h>
#include <Spark/Types.h>
#include <string>
#include <unordered_map>
#include <charconv>
#include <cstdint>
#include <cstdio>
#include <system_error>
#include <type_traits>


//报尾固定为 "5=" + CRC32C 的 8 位十六进制 + SOH
constexpr unsigned int StepTailLen = 2u + 8u + 1u;
//包头由 key=value 字段串成、长度不固定，这个值只是"超过它还没解析出包头就判定为非法"的上界
constexpr unsigned int StepMaxHeaderLen = 128u;
constexpr unsigned int SOH = 1u;

//WriteString 的重载形参一律用调色板别名，重载集合与 Types.h 的别名一一对应。代价是别名改了类型时重载照样匹配、
//只有 .cpp 里的格式串会失配，所以把格式串绑在具体类型上的那四个别名在这里钉死；其余重载体内先 static_cast 到
//int / unsigned int，或靠默认提升，别名怎么变都不会失配。Types.h 的宽度断言管不住这件事——sizeof(int64_t) 也是 8，
//Linux 上它就是 long。本套重载只覆盖调色板里的类型，传裸 long / size_t 会落到末尾那个模板重载上按 %s 把整数
//当 char* 解引用
static_assert(std::is_same<Int32Type, int>::value, "Int32Type 必须是 int，%d 才匹配");
static_assert(std::is_same<Int64Type, long long>::value, "Int64Type 必须是 long long，%lld 才匹配");
static_assert(std::is_same<UInt64Type, unsigned long long>::value, "UInt64Type 必须是 unsigned long long，%llu 才匹配");
static_assert(std::is_same<DoubleType, double>::value, "DoubleType 必须是 double，%.6f 才匹配");

namespace spark::network
{
class NETWORK_EXPORTS StepUtility
{
public:
	static bool GetNextSoh(char* buff, int startIndex, int endIndex, int& sohIndex);
	static bool GetNextEqual(char* buff, int startIndex, int endIndex, int& equalIndex);
	static bool GetNext(char* buff, int startIndex, int endIndex, uint16_t& key, std::string& value, int& sohIndex);
	static bool GetFieldStart(char* buff, int startIndex, int endIndex, uint16_t& fieldID, int& fieldStartIndex);
	static bool GetFieldEnd(char* buff, int startIndex, int endIndex, uint16_t& fieldID, int& fieldEndIndex);
	static bool GetNextFieldZone(char* buff, int startIndex, int endIndex, uint16_t& fieldID, int& fieldStartIndex, int& fieldEndIndex);
	//报文起始锚点，形如 SOH + "0=SPK2" + SOH，与 ProtocolVersion.h 的魔术字是同一串字节
	static const std::string& GetPackageStartAnchor();

	static void WriteString(char*& ppos, int key, BoolType value);
	static void WriteString(char*& ppos, int key, char value);
	static void WriteString(char*& ppos, int key, Int8Type value);
	static void WriteString(char*& ppos, int key, UInt8Type value);
	static void WriteString(char*& ppos, int key, Int16Type value);
	static void WriteString(char*& ppos, int key, UInt16Type value);
	static void WriteString(char*& ppos, int key, Int32Type value);
	static void WriteString(char*& ppos, int key, UInt32Type value);
	static void WriteString(char*& ppos, int key, Int64Type value);
	static void WriteString(char*& ppos, int key, UInt64Type value);
	static void WriteString(char*& ppos, int key, DoubleType value);
	static void WriteString(char*& ppos, int key, std::string value);
	static void WriteString(char*& ppos, int key, char* value);

	//文本转整型：格式非法或越界都返回 false。窄类型直接 atoi 会静默截断，所以必须走这里
	template<typename T>
	static bool ParseInteger(const std::string& text, T& value)
	{
		static_assert(std::is_integral<T>::value, "ParseInteger 只接受整型");
		if (text.empty())
		{
			return false;
		}
		T parsed = 0;
		const char* first = text.data();
		const char* last = first + text.size();
		auto result = std::from_chars(first, last, parsed);
		if (result.ec != std::errc() || result.ptr != last)
		{
			return false;
		}
		value = parsed;
		return true;
	}
	template<typename T>
	static void WriteString(char*& ppos, int key, T value)
	{
		int len = sprintf(ppos, "%d=%s", key, value);
		ppos += len;
		*ppos++ = SOH;
	}
	static void WriteHexString(char*& ppos, int key, uint16_t value);


	//返回实际写入的包头长度
	static int HeadToStream(HeadField* head, char* buff, int size);
	//成功时把包头结束位置写入 headEndIndex，即包体的起始位置
	static bool HeadFromStream(char* buff, int startIndex, int endIndex, HeadField* head, int& headEndIndex);
	//返回实际写入的报尾长度
	static int TailToStream(TailField* tail, char* buff, int size);
	static bool TailFromStream(char* buff, int startIndex, int endIndex, TailField* tail);

};
}
