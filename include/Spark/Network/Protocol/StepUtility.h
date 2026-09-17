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


//协议里所有键都固定 4 位大写十六进制，后跟 '='。键宽是线上格式的一部分，读写两侧都取这个值
constexpr int StepKeyTextLen = 4;
//包头由定宽字段拼成，长度与取值无关，故为编译期常量。每一段都用 StepKeyTextLen 表达，这样键宽一变
//下面那条 static_assert 就会立刻报红，而不是留下"写侧发 4 位键、读侧找 5 位键"的静默错位。
//锚点 1+4+1+4+1=11、Version/PackageId/BodyLen 各 4+1+4+1=10、MsgSeqNum 4+1+8+1=14、MessageChain 4+1+1+1=7
constexpr unsigned int StepHeadLen = (1u + StepKeyTextLen + 1u + 4u + 1u)
    + (StepKeyTextLen + 1u + 4u + 1u) * 3u
    + (StepKeyTextLen + 1u + 8u + 1u)
    + (StepKeyTextLen + 1u + 1u + 1u);
//报尾固定为 "0005=" + CRC32C 的 8 位十六进制 + SOH
constexpr unsigned int StepTailLen = StepKeyTextLen + 1u + 8u + 1u;
//单侧缓冲区还没收满这么多字节就无法判定包头，读者据此决定"继续等数据"而不是"判定非法"。
//写侧不用它：包头已定长为 StepHeadLen
constexpr unsigned int StepMaxHeaderLen = 128u;
constexpr char SOH = '\x01';

static_assert(StepHeadLen == 62u, "包头布局变化必须同步核对 HeadToStream 的字段格式与宽度");
static_assert(StepTailLen == 14u, "报尾布局变化必须同步核对 TailToStream 的字段格式与宽度");

static_assert(std::is_same<Int32Type, int>::value, "Int32Type 必须是 int，XTP 按 sizeof 取宽");
static_assert(std::is_same<Int64Type, long long>::value, "Int64Type 必须是 long long，XTP 按 sizeof 取宽");
static_assert(std::is_same<UInt64Type, unsigned long long>::value, "UInt64Type 必须是 unsigned long long，XTP 按 sizeof 取宽");
static_assert(std::is_same<DoubleType, double>::value, "DoubleType 必须是 double，XTP 按 sizeof 取宽");

namespace Spark::Network
{
class NETWORK_EXPORTS StepWriteCursor
{
public:
    StepWriteCursor(char* buffer, int capacity);
    int GetWrittenLength() const;
    bool IsTruncated() const;

    template<typename... FieldValues>
    bool AppendField(std::format_string<FieldValues...> fieldFormat, FieldValues&&... fieldValues)
    {
        if (isTruncated_)
        {
            return false;
        }
        const int remainingLength = GetRemainingLength();
        if (remainingLength <= 0)
        {
            isTruncated_ = true;
            return false;
        }
        const int writableLength = remainingLength - 1;
        auto result = std::format_to_n(GetWritePosition(), static_cast<std::size_t>(writableLength), fieldFormat,
            std::forward<FieldValues>(fieldValues)...);
        if (result.size > static_cast<std::ptrdiff_t>(writableLength))
        {
            isTruncated_ = true;
            return false;
        }
        writtenLength_ += static_cast<int>(result.out - GetWritePosition());
        *GetWritePosition() = SOH;
        writtenLength_ += 1;
        return true;
    }

private:
    char* GetWritePosition();
    int GetRemainingLength() const;

    char* bufferBegin_;
    int capacity_;
    int writtenLength_;
    bool isTruncated_;
};

class NETWORK_EXPORTS StepUtility
{
public:
    static bool GetNextSoh(char* buff, int startIndex, int endIndex, int& sohIndex);
    static bool GetNextEqual(char* buff, int startIndex, int endIndex, int& equalIndex);
    static bool GetNext(char* buff, int startIndex, int endIndex, UInt16Type& key, std::string& value, int& sohIndex);
    static bool GetFieldStart(char* buff, int startIndex, int endIndex, UInt16Type& fieldId, int& fieldStartIndex);
    static bool GetFieldEnd(char* buff, int startIndex, int endIndex, UInt16Type& fieldId, int& fieldEndIndex);
    static bool GetNextFieldZone(char* buff, int startIndex, int endIndex, UInt16Type& fieldId, int& fieldStartIndex, int& fieldEndIndex);
    //报文起始锚点，形如 SOH + "0000=SPK2" + SOH，与 ProtocolVersion.h 的魔术字是同一串字节，
    //且与 HeadToStream 写出的首字段逐字节相同（两处共用同一次格式化）
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

    //文本转整型：格式非法或越界都返回 false。窄类型直接 atoi 会静默截断，所以必须走这里。
    //base 给 16 时 from_chars 对无符号目标会拒绝 '-'，越界则报 result_out_of_range，无需再手写上界
    template<typename T>
    static bool ParseInteger(const std::string& text, T& value, int base = 10)
    {
        static_assert(std::is_integral<T>::value, "ParseInteger 只接受整型");
        if (text.empty())
        {
            return false;
        }
        T parsed = 0;
        const char* first = text.data();
        const char* last = first + text.size();
        auto result = std::from_chars(first, last, parsed, base);
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
        cursor.AppendField("{:04X}={:s}", key, value);
    }
    static void WriteHexString(StepWriteCursor& cursor, UInt16Type key, UInt16Type value);


    //写入定长包头。成功返回 StepHeadLen，size 不足 StepHeadLen 或字段格式写漏都返回 0
    static int HeadToStream(HeadField* head, char* buff, int size);
    //成功时把包头结束位置写入 headEndIndex，即包体的起始位置
    static bool HeadFromStream(char* buff, int startIndex, int endIndex, HeadField* head, int& headEndIndex);
    //写入定长报尾。成功返回 StepTailLen，size 不足或格式化长度不符都返回 0
    static int TailToStream(TailField* tail, char* buff, int size);
    static bool TailFromStream(char* buff, int startIndex, int endIndex, TailField* tail);

};
}
