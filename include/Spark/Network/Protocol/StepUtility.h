#pragma once
#include <Spark/Network/NetworkExport.h>
#include <Spark/Network/Protocol/Head.h>
#include <Spark/Types.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <format>
#include <system_error>
#include <type_traits>


//报尾固定为 "5=" + CRC32C 的 8 位十六进制 + SOH
constexpr unsigned int StepTailLen = 2u + 8u + 1u;
//包头由 key=value 字段串成、长度不固定，这个值只是"超过它还没解析出包头就判定为非法"的上界
constexpr unsigned int StepMaxHeaderLen = 128u;
constexpr char SOH = '\x01';

//WriteString 的重载形参一律用调色板别名，重载集合与 Types.h 的别名一一对应。格式串改由 std::format_string<Args...>
//在编译期逐字段校验，但 {:d} 对任何整型宽度都成立，所以别名宽度漂移不归它管：兜底的是"重载集合必须两两不同型"
//（两个别名撞成同一种类型即重定义报错）加上下面那几条钉类型同一性的 static_assert——别名漂移成同宽异型时
//std::format 不响、Types.h 的几条 sizeof 断言也不响（它断言的是字面类型 long long / double / bool，不引用别名），
//而 XTP 路径按 sizeof(别名) 走 memcpy，宽度一变线上格式就错位。本套重载只覆盖调色板里的类型与字符串指针，
//传裸 long / size_t 会落到末尾那个模板重载上，由它 static_assert 拒绝
static_assert(std::is_same<Int32Type, int>::value, "Int32Type 必须是 int，XTP 按 sizeof 取宽");
static_assert(std::is_same<Int64Type, long long>::value, "Int64Type 必须是 long long，XTP 按 sizeof 取宽");
static_assert(std::is_same<UInt64Type, unsigned long long>::value, "UInt64Type 必须是 unsigned long long，XTP 按 sizeof 取宽");
static_assert(std::is_same<DoubleType, double>::value, "DoubleType 必须是 double，XTP 按 sizeof 取宽");

namespace spark::network
{
//把 STEP 字段正文按容量上界写入缓冲并补 SOH。容量里含为 SOH 预留的 1 字节，放不下时不推进写游标、
//置截断标志并返回 false，且此后不再接受任何字段写入；调用方据此回一个负长度，落到 Package::MakePackage
//已有的 bodyLen < 0 判定上。由此 [begin, begin + GetWrittenLength()) 始终是完整的字段序列，
//但 std::format_to_n 会把能放下的前缀留在该区间之后，所以截断时调用方必须丢弃整个包体
class NETWORK_EXPORTS StepWriteCursor
{
public:
	StepWriteCursor(char* buffer, int capacity);

	char* GetWritePosition() const;
	int GetRemainingLength() const;
	int GetWrittenLength() const;
	bool IsTruncated() const;

	template<typename... FieldValues>
	bool AppendField(std::format_string<FieldValues...> fieldFormat, FieldValues&&... fieldValues)
	{
		if (is_truncated_)
		{
			return false;
		}
		const int remainingLength = GetRemainingLength();
		if (remainingLength <= 0)
		{
			is_truncated_ = true;
			return false;
		}
		const int writableLength = remainingLength - 1;
		auto result = std::format_to_n(GetWritePosition(), static_cast<std::size_t>(writableLength), fieldFormat,
			std::forward<FieldValues>(fieldValues)...);
		if (result.size > static_cast<std::ptrdiff_t>(writableLength))
		{
			is_truncated_ = true;
			return false;
		}
		written_length_ += static_cast<int>(result.out - GetWritePosition());
		*GetWritePosition() = SOH;
		written_length_ += 1;
		return true;
	}

private:
	char* buffer_begin_;
	int capacity_;
	int written_length_;
	bool is_truncated_;
};

class NETWORK_EXPORTS StepUtility
{
public:
	static bool GetNextSoh(char* buff, int startIndex, int endIndex, int& sohIndex);
	static bool GetNextEqual(char* buff, int startIndex, int endIndex, int& equalIndex);
	static bool GetNext(char* buff, int startIndex, int endIndex, UInt16Type& key, std::string& value, int& sohIndex);
	static bool GetFieldStart(char* buff, int startIndex, int endIndex, UInt16Type& fieldID, int& fieldStartIndex);
	static bool GetFieldEnd(char* buff, int startIndex, int endIndex, UInt16Type& fieldID, int& fieldEndIndex);
	static bool GetNextFieldZone(char* buff, int startIndex, int endIndex, UInt16Type& fieldID, int& fieldStartIndex, int& fieldEndIndex);
	//报文起始锚点，形如 SOH + "0=SPK2" + SOH，与 ProtocolVersion.h 的魔术字是同一串字节
	static const std::string& GetPackageStartAnchor();

	static void WriteString(StepWriteCursor& cursor, UInt16Type key, BoolType value);
	static void WriteString(StepWriteCursor& cursor, UInt16Type key, char value);
	static void WriteString(StepWriteCursor& cursor, UInt16Type key, Int8Type value);
	static void WriteString(StepWriteCursor& cursor, UInt16Type key, UInt8Type value);
	static void WriteString(StepWriteCursor& cursor, UInt16Type key, Int16Type value);
	static void WriteString(StepWriteCursor& cursor, UInt16Type key, UInt16Type value);
	static void WriteString(StepWriteCursor& cursor, UInt16Type key, Int32Type value);
	static void WriteString(StepWriteCursor& cursor, UInt16Type key, UInt32Type value);
	static void WriteString(StepWriteCursor& cursor, UInt16Type key, Int64Type value);
	static void WriteString(StepWriteCursor& cursor, UInt16Type key, UInt64Type value);
	static void WriteString(StepWriteCursor& cursor, UInt16Type key, DoubleType value);
	static void WriteString(StepWriteCursor& cursor, UInt16Type key, std::string value);

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
	static void WriteString(StepWriteCursor& cursor, UInt16Type key, T value)
	{
		static_assert(std::is_same<T, const char*>::value || std::is_same<T, char*>::value,
			"WriteString 只覆盖 Types.h 调色板里的类型与字符串指针；裸 long / size_t 请先转成对应别名");
		cursor.AppendField("{}={:s}", key, value);
	}
	static void WriteHexString(StepWriteCursor& cursor, UInt16Type key, UInt16Type value);


	//返回实际写入的包头长度
	static int HeadToStream(HeadField* head, char* buff, int size);
	//成功时把包头结束位置写入 headEndIndex，即包体的起始位置
	static bool HeadFromStream(char* buff, int startIndex, int endIndex, HeadField* head, int& headEndIndex);
	//返回实际写入的报尾长度
	static int TailToStream(TailField* tail, char* buff, int size);
	static bool TailFromStream(char* buff, int startIndex, int endIndex, TailField* tail);

};
}
