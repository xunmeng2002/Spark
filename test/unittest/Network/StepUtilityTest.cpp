#include <gtest/gtest.h>
#include <Spark/Network/Protocol/StepUtility.h>
#include <Spark/Network/Protocol/Head.h>
#include <Spark/Network/Protocol/Items.h>
#include <Spark/Network/Protocol/ProtocolUtility.h>
#include <Spark/Network/Protocol/ProtocolVersion.h>

#include <cstring>
#include <format>
#include <limits>
#include <string>
#include <vector>
using namespace Spark::Network;
// ============================================================
// StepUtility 测试
// STEP 协议缓冲解析/序列化：字段定位、报文头/尾流式转换
// ============================================================

namespace
{
    // 构造 "key=value\SOH"（无前导 SOH，用于 GetNext 直接解析）。
    // 键在线上是 4 位大写十六进制，这里与 StepUtility::WriteString 的写法保持一致
    std::string MakeStepField(unsigned int key, const std::string& value)
    {
        return std::format("{:04X}", key) + "=" + value + std::string(1, SOH);
    }

    //包头用例里"合法版本号"的唯一取值。写成常量，版本一升就不必逐处翻找占位串
    const std::string kStepVersionText = std::format("{:04X}", ProtocolVersionValue);

    //写入路径用例骨架：搭缓冲 → 建游标 → 写一个字段 → 与预期字段串逐字节比对。
    //容量取 64：写入路径的字段都是短文本或单个数字；容量边界与截断语义另有专门用例，不走这里
    template<typename FieldValue>
    void ExpectStepField(UInt16Type key, const FieldValue& value, const std::string& expectedValue)
    {
        char buff[64] = {};
        StepWriteCursor cursor(buff, static_cast<int>(sizeof(buff)));
        StepUtility::WriteString(cursor, key, value);

        EXPECT_EQ(std::string(buff, cursor.GetWrittenLength()), MakeStepField(key, expectedValue));
    }

    void ExpectHexStepField(UInt16Type key, UInt16Type value, const std::string& expectedValue)
    {
        char buff[64] = {};
        StepWriteCursor cursor(buff, static_cast<int>(sizeof(buff)));
        StepUtility::WriteHexString(cursor, key, value);

        EXPECT_EQ(std::string(buff, cursor.GetWrittenLength()), MakeStepField(key, expectedValue));
    }

    //只校验键的写法：4 位大写十六进制 + '='。键的格式串在 14 个写入入口各写一次，
    //没有公共 helper 可抽，所以逐个遍历——任何一个入口漏改都会先红。
    //键取 0x00Ax，让补零与大写字母同时可见
    template<typename FieldValue>
    void ExpectStepKeyWritten(UInt16Type key, const FieldValue& value)
    {
        char buff[64] = {};
        StepWriteCursor cursor(buff, static_cast<int>(sizeof(buff)));
        StepUtility::WriteString(cursor, key, value);

        ASSERT_GE(cursor.GetWrittenLength(), StepKeyTextLen + 1);
        EXPECT_EQ(std::string(buff, buff + StepKeyTextLen), std::format("{:04X}", key));
        EXPECT_EQ(buff[StepKeyTextLen], '=');
    }

    // 按 HeadToStream 的写法造一个完整包头，返回实际写出的字节
    std::string MakeStepHeadStream(UInt16Type packageId, UInt16Type bodyLen, UInt32Type msgSeqNum, BoolType messageChain)
    {
        HeadField head = {};
        head.Magic = ProtocolMagicValue;
        head.Version = ProtocolVersionValue;
        head.PackageId = packageId;
        head.BodyLen = bodyLen;
        head.MsgSeqNum = msgSeqNum;
        head.MessageChain = messageChain;

        char buff[StepHeadLen] = {};
        int len = StepUtility::HeadToStream(&head, buff, StepHeadLen);
        return std::string(buff, buff + len);
    }

    // 手工拼包头，只留出字段值可控的口子，便于构造坏值用例。
    // 各字段的默认值都取合法十六进制文本，用例只覆盖自己关心的那一个
    std::string MakeRawStepHeadStream(
        const std::string& magic = ProtocolMagicText,
        const std::string& version = kStepVersionText,
        const std::string& packageId = "1001",
        const std::string& bodyLen = "0008",
        const std::string& msgSeqNum = "00000007",
        const std::string& messageChain = "0",
        const std::string& trailing = "")
    {
        return std::string(1, SOH)
            + MakeStepField(Items::Magic, magic)
            + MakeStepField(Items::Version, version)
            + MakeStepField(Items::PackageId, packageId)
            + MakeStepField(Items::BodyLen, bodyLen)
            + MakeStepField(Items::MsgSeqNum, msgSeqNum)
            + MakeStepField(Items::MessageChain, messageChain)
            + trailing;
    }

    // 按 TailToStream 的写法造一个完整报尾，返回实际写出的字节
    std::string MakeStepTailStream(UInt32Type checkSum)
    {
        TailField tail = {};
        tail.CheckSum = checkSum;

        char buff[StepTailLen + 1] = {};
        StepUtility::TailToStream(&tail, buff, StepTailLen);
        return std::string(buff, buff + StepTailLen);
    }
}

// ============================================================
// GetNextSoh
// ============================================================

TEST(StepUtilityTest, GetNextSoh_Found)
{
    char buff[] = { 'a', 'b', SOH, 'c', 'd' };
    int sohIndex = -1;
    EXPECT_TRUE(StepUtility::GetNextSoh(buff, 0, 5, sohIndex));
    EXPECT_EQ(sohIndex, 2);
}

TEST(StepUtilityTest, GetNextSoh_NotFound)
{
    char buff[] = { 'a', 'b', 'c', 'd' };
    int sohIndex = -1;
    EXPECT_FALSE(StepUtility::GetNextSoh(buff, 0, 4, sohIndex));
}

TEST(StepUtilityTest, GetNextSoh_EmptyRange)
{
    char buff[] = { SOH, 'a' };
    int sohIndex = -1;
    EXPECT_FALSE(StepUtility::GetNextSoh(buff, 2, 2, sohIndex));
}

// ============================================================
// GetNextEqual
// ============================================================

TEST(StepUtilityTest, GetNextEqual_Found)
{
    char buff[] = { '1', '=', '2', SOH };
    int equalIndex = -1;
    EXPECT_TRUE(StepUtility::GetNextEqual(buff, 0, 4, equalIndex));
    EXPECT_EQ(equalIndex, 1);
}

TEST(StepUtilityTest, GetNextEqual_NotFound)
{
    char buff[] = { '1', '2', '3', SOH };
    int equalIndex = -1;
    EXPECT_FALSE(StepUtility::GetNextEqual(buff, 0, 4, equalIndex));
}

// ============================================================
// GetNext
// ============================================================

TEST(StepUtilityTest, GetNext_SimpleField)
{
    // GetNext 期望格式: key=value\SOH（无前导 SOH）
    std::string field = MakeStepField(1, "0001");
    unsigned short key = 0;
    std::string value;
    int sohIndex = -1;

    EXPECT_TRUE(StepUtility::GetNext(&field[0], 0, (int)field.size(), key, value, sohIndex));
    EXPECT_EQ(key, 1);
    EXPECT_EQ(value, "0001");
}

TEST(StepUtilityTest, GetNext_StringValue)
{
    std::string field = MakeStepField(0x100D, "600001");
    unsigned short key = 0;
    std::string value;
    int sohIndex = -1;

    EXPECT_TRUE(StepUtility::GetNext(&field[0], 0, (int)field.size(), key, value, sohIndex));
    EXPECT_EQ(key, 0x100D);
    EXPECT_EQ(value, "600001");
}

TEST(StepUtilityTest, GetNext_NoSoh)
{
    char buff[] = "1=0001";  // no SOH
    unsigned short key = 0;
    std::string value;
    int sohIndex = -1;

    EXPECT_FALSE(StepUtility::GetNext(buff, 0, 6, key, value, sohIndex));
}

TEST(StepUtilityTest, GetNext_NoEqual)
{
    // 有 SOH 但在它之前没有 '='
    char buff[] = { '1', 'a', 'b', SOH };
    unsigned short key = 0;
    std::string value;
    int sohIndex = -1;

    EXPECT_FALSE(StepUtility::GetNext(buff, 0, 4, key, value, sohIndex));
}

TEST(StepUtilityTest, GetNext_NonHexKey)
{
    // 键含非十六进制字符必须判失败。旧实现走 atoi，对非法串静默返回 0，
    // 畸形帧会被当成 "0=..." 的合法字段
    std::string field = std::string("000G=1") + std::string(1, SOH);
    unsigned short key = 0xFFFF;
    std::string value;
    int sohIndex = -1;

    EXPECT_FALSE(StepUtility::GetNext(&field[0], 0, static_cast<int>(field.size()), key, value, sohIndex));
}

TEST(StepUtilityTest, GetNext_KeyOutOfRange)
{
    // 5 位十六进制超过 16 位键宽，必须判失败而不是截断成 0x0000
    std::string field = std::string("10000=1") + std::string(1, SOH);
    unsigned short key = 0;
    std::string value;
    int sohIndex = -1;

    EXPECT_FALSE(StepUtility::GetNext(&field[0], 0, static_cast<int>(field.size()), key, value, sohIndex));
}

// ============================================================
// 报文起始锚点（SOH + "0000=SPK2" + SOH）
// ============================================================

TEST(StepUtilityTest, PackageStartAnchor_Format)
{
    const std::string& anchor = StepUtility::GetPackageStartAnchor();
    // SOH + "0000" + "=" + "SPK2" + SOH，共 11 字节（= 包首字段的字节数）
    ASSERT_EQ(anchor.size(), 11u);
    EXPECT_EQ(anchor[0], SOH);
    EXPECT_EQ(anchor.substr(1, 4), "0000");
    EXPECT_EQ(anchor[5], '=');
    EXPECT_EQ(anchor.substr(6, 4), "SPK2");
    EXPECT_EQ(anchor[10], SOH);
}

TEST(StepUtilityTest, PackageStartAnchor_MatchesMagicValueBytes)
{
    // 锚点里的 "SPK2" 必须与 ProtocolMagicValue 是小端的同一串字节
    const std::string& anchor = StepUtility::GetPackageStartAnchor();
    unsigned int magic = 0;
    memcpy(&magic, anchor.data() + 6, 4);
    EXPECT_EQ(magic, static_cast<unsigned int>(ProtocolMagicValue));
}

TEST(StepUtilityTest, PackageStartAnchor_MatchesHeadToStream)
{
    // 序列化出来的包头必须以锚点开头，否则接收端永远对不上
    std::string headStream = MakeStepHeadStream(0x1001, 0, 1, 0);
    const std::string& anchor = StepUtility::GetPackageStartAnchor();
    EXPECT_EQ(headStream.compare(0, anchor.size(), anchor), 0) << headStream;
}

// ============================================================
// GetFieldStart / GetFieldEnd / GetNextFieldZone
//（需要前导 SOH 后跟 "0006=" / "0007="）
// ============================================================

TEST(StepUtilityTest, GetFieldStart_Found)
{
    // FieldStart marker: SOH + "0006=" + hexFieldId
    // 0006=100D indicates field 0x100D
    std::string data = std::string(1, SOH) + MakeStepField(6, "100D")
                     + "some_data"
                     + std::string(1, SOH) + MakeStepField(7, "100D");
    unsigned short fieldId = 0;
    int startIndex = -1;

    EXPECT_TRUE(StepUtility::GetFieldStart(&data[0], 0, (int)data.size(), fieldId, startIndex));
    EXPECT_EQ(fieldId, 0x100D);
    EXPECT_GE(startIndex, 0);
}

TEST(StepUtilityTest, GetFieldStart_NotFound)
{
    std::string data = std::string(1, SOH) + MakeStepField(2, "0005");  // no field start marker
    unsigned short fieldId = 0;
    int startIndex = -1;

    EXPECT_FALSE(StepUtility::GetFieldStart(&data[0], 0, (int)data.size(), fieldId, startIndex));
}

TEST(StepUtilityTest, GetFieldEnd_Found)
{
    std::string data = std::string(1, SOH) + MakeStepField(7, "100D");
    unsigned short fieldId = 0;
    int endIndex = -1;

    EXPECT_TRUE(StepUtility::GetFieldEnd(&data[0], 0, (int)data.size(), fieldId, endIndex));
    EXPECT_EQ(fieldId, 0x100D);
}

TEST(StepUtilityTest, GetNextFieldZone_Complete)
{
    std::string data = std::string(1, SOH) + MakeStepField(6, "100D")
                     + "content"
                     + std::string(1, SOH) + MakeStepField(7, "100D");
    unsigned short fieldId = 0;
    int startIdx = -1, endIdx = -1;

    EXPECT_TRUE(StepUtility::GetNextFieldZone(&data[0], 0, (int)data.size(), fieldId, startIdx, endIdx));
    EXPECT_EQ(fieldId, 0x100D);
    EXPECT_GE(startIdx, 0);
    EXPECT_GT(endIdx, startIdx);
}

TEST(StepUtilityTest, GetNextFieldZone_MismatchedIds)
{
    // FieldId 和 FieldEnd 的 Id 不匹配
    std::string data = std::string(1, SOH) + MakeStepField(6, "100D")
                     + "content"
                     + std::string(1, SOH) + MakeStepField(7, "100E");
    unsigned short fieldId = 0;
    int startIdx = -1, endIdx = -1;

    EXPECT_FALSE(StepUtility::GetNextFieldZone(&data[0], 0, (int)data.size(), fieldId, startIdx, endIdx));
}

TEST(StepUtilityTest, GetFieldStart_MaxFieldId)
{
    //0xFFFF 是 16 位字段 Id 的上边界，必须仍然合法；范围检查写成越界一个会误拒它
    std::string data = std::string(1, SOH) + MakeStepField(6, "FFFF");
    uint16_t fieldId = 0;
    int startIndex = -1;

    EXPECT_TRUE(StepUtility::GetFieldStart(&data[0], 0, (int)data.size(), fieldId, startIndex));
    EXPECT_EQ(fieldId, 0xFFFF);
}

TEST(StepUtilityTest, GetFieldStart_FieldIdOutOfRange)
{
    //0x10000 超出 16 位，截断后是 0x0000，会让畸形帧被当成合法字段
    std::string data = std::string(1, SOH) + MakeStepField(6, "10000");
    uint16_t fieldId = 0;
    int startIndex = -1;

    EXPECT_FALSE(StepUtility::GetFieldStart(&data[0], 0, (int)data.size(), fieldId, startIndex));
}

TEST(StepUtilityTest, GetNextFieldZone_FieldEndIdOutOfRange)
{
    //结束 Id 0x1100D 截断后是 0x100D，与起始 Id 相等，会让结束 Id 越界的畸形帧通过 Id 校验
    std::string data = std::string(1, SOH) + MakeStepField(6, "100D")
                     + "content"
                     + std::string(1, SOH) + MakeStepField(7, "1100D");
    uint16_t fieldId = 0;
    int startIdx = -1, endIdx = -1;

    EXPECT_FALSE(StepUtility::GetNextFieldZone(&data[0], 0, (int)data.size(), fieldId, startIdx, endIdx));
}

// ============================================================
// WriteString（所有重载）
// ============================================================

TEST(StepUtilityTest, WriteString_Bool)
{
    ExpectStepField(0x8001, true, "1");   // IsAllowLogin
}

TEST(StepUtilityTest, WriteString_Char)
{
    ExpectStepField(0x9005, 'B', "B");    // Direction
}

TEST(StepUtilityTest, WriteString_UnsignedShort)
{
    ExpectStepField(0x0001, static_cast<UInt16Type>(0x0001), "1");  // PackageId
}

TEST(StepUtilityTest, WriteString_Int)
{
    ExpectStepField(0x0004, 123456, "123456");  // MsgSeqNum
}

TEST(StepUtilityTest, WriteString_LongLong)
{
    ExpectStepField(0x3001, 20240115LL, "20240115");  // TradingDay
}

//钉住 UInt64Type 必须命中 %llu 重载：落到末尾那个 template 重载会按 %s 把整数当 char* 解引用
TEST(StepUtilityTest, WriteString_UInt64)
{
    //0x9999 不占用任何 ItemId：模型里没有 UInt64 字段，本用例只验证重载分派
    //取 UInt64 上限，确保不会被 32 位重载静默截断
    ExpectStepField(0x9999, static_cast<UInt64Type>(18446744073709551615ULL), "18446744073709551615");
}

TEST(StepUtilityTest, WriteString_Double)
{
    ExpectStepField(0x6015, 12.345, "12.345000");  // Price
}

TEST(StepUtilityTest, WriteString_StdString)
{
    ExpectStepField(0x100D, std::string("600001"), "600001");  // InstrumentId
}

TEST(StepUtilityTest, WriteString_CharPtr)
{
    ExpectStepField(0x100D, "cu2401", "cu2401");
}

TEST(StepUtilityTest, WriteHexString)
{
    ExpectHexStepField(0x0001, 0x00FF, "00FF");  // test with value 255
}

//bool 落到 std::format 的 {} 会输出 true/false，必须显式 {:d} 才与 sprintf 时代的 0/1 逐字节一致
TEST(StepUtilityTest, WriteString_BoolStaysNumeric)
{
    ExpectStepField(0x8001, false, "0");
}

//Int8Type / UInt8Type 是 signed char / unsigned char，不在 std::format 的字符类型集合内，{:d} 给整数
TEST(StepUtilityTest, WriteString_Int8)
{
    ExpectStepField(0x0002, static_cast<Int8Type>(-5), "-5");
}

TEST(StepUtilityTest, WriteString_UInt8)
{
    ExpectStepField(0x0003, static_cast<UInt8Type>(200), "200");
}

//键宽守卫：遍历 WriteString 的全部 12 个类型重载、字符串指针重载与 WriteHexString，
//任一入口的键格式漏改成 {:04X} 都会在这里现形
TEST(StepUtilityTest, WriteKeyWidthIsFourDigitHexForAllOverloads)
{
    ExpectStepKeyWritten(static_cast<UInt16Type>(0x00A1), true);                              // BoolType
    ExpectStepKeyWritten(static_cast<UInt16Type>(0x00A2), 'B');                               // char
    ExpectStepKeyWritten(static_cast<UInt16Type>(0x00A3), static_cast<Int8Type>(-5));         // Int8Type
    ExpectStepKeyWritten(static_cast<UInt16Type>(0x00A4), static_cast<UInt8Type>(200));       // UInt8Type
    ExpectStepKeyWritten(static_cast<UInt16Type>(0x00A5), static_cast<Int16Type>(-1234));     // Int16Type
    ExpectStepKeyWritten(static_cast<UInt16Type>(0x00A6), static_cast<UInt16Type>(54321));    // UInt16Type
    ExpectStepKeyWritten(static_cast<UInt16Type>(0x00A7), static_cast<Int32Type>(-123456));   // Int32Type
    ExpectStepKeyWritten(static_cast<UInt16Type>(0x00A8), static_cast<UInt32Type>(4294967295u)); // UInt32Type
    ExpectStepKeyWritten(static_cast<UInt16Type>(0x00A9), static_cast<Int64Type>(-1));        // Int64Type
    ExpectStepKeyWritten(static_cast<UInt16Type>(0x00AA), static_cast<UInt64Type>(1));        // UInt64Type
    ExpectStepKeyWritten(static_cast<UInt16Type>(0x00AB), 12.345);                            // DoubleType
    ExpectStepKeyWritten(static_cast<UInt16Type>(0x00AC), std::string("600001"));             // std::string
    ExpectStepKeyWritten(static_cast<UInt16Type>(0x00AD), "cu2401");                          // const char*

    char buff[64] = {};
    StepWriteCursor cursor(buff, static_cast<int>(sizeof(buff)));
    StepUtility::WriteHexString(cursor, 0x00AE, 0x1234);
    EXPECT_EQ(std::string(buff, buff + StepKeyTextLen), "00AE");
    EXPECT_EQ(buff[StepKeyTextLen], '=');
}

//容量正好等于 "key=value" + SOH：不判截断，落盘字节与 MakeStepField 完全一致
TEST(StepUtilityTest, WriteString_ExactCapacityFits)
{
    const std::string expected = MakeStepField(0x8001, "1");
    std::vector<char> buff(expected.size(), 'X');
    StepWriteCursor cursor(&buff[0], static_cast<int>(buff.size()));

    StepUtility::WriteString(cursor, 0x8001, true);

    EXPECT_FALSE(cursor.IsTruncated());
    EXPECT_EQ(cursor.GetWrittenLength(), static_cast<int>(expected.size()));
    EXPECT_EQ(std::string(&buff[0], buff.size()), expected);
}

//容量少 1 字节：正文放得下、SOH 放不下，必须判截断且不推进写游标
TEST(StepUtilityTest, WriteString_ReservesByteForSoh)
{
    const std::string expected = MakeStepField(0x8001, "1");
    std::vector<char> buff(expected.size() - 1, 'X');
    StepWriteCursor cursor(&buff[0], static_cast<int>(buff.size()));

    StepUtility::WriteString(cursor, 0x8001, true);

    EXPECT_TRUE(cursor.IsTruncated());
    //写游标不推进，所以长度仍是 0；format_to_n 会把放得下的前缀留在缓冲里，调用方据截断标志丢弃整个包体
    EXPECT_EQ(cursor.GetWrittenLength(), 0);
    //容量的最后一字节是为 SOH 预留的，从未交给格式化器
    EXPECT_EQ(buff[expected.size() - 2], 'X');
}

//容量非正不得越界写、也不得让容量减法自身溢出：ToStepStream 是 Package 的公开虚函数，
//仓外调用方可传任意 size（含 INT_MIN）。仓内走的 Step 分支已保证 bodyCapacity >= 1
TEST(StepUtilityTest, WriteString_NonPositiveCapacityTruncates)
{
    char buff[8] = {};
    for (int capacity : { 0, -1, std::numeric_limits<int>::min() })
    {
        StepWriteCursor cursor(buff, capacity);
        StepUtility::WriteString(cursor, 0x8001, true);

        EXPECT_TRUE(cursor.IsTruncated());
        EXPECT_EQ(cursor.GetWrittenLength(), 0);
    }
    EXPECT_EQ(std::string(buff, sizeof(buff)), std::string(sizeof(buff), '\0'));
}

//第一次截断之后再写短字段也必须被拒：否则缓冲里会留下"半截字段 + 后续字段"的拼接，事后无法分辨
TEST(StepUtilityTest, WriteCursor_StopsAfterTruncation)
{
    char buff[16] = {};
    StepWriteCursor cursor(buff, static_cast<int>(sizeof(buff)));

    EXPECT_FALSE(cursor.AppendField("{}={:s}", static_cast<UInt16Type>(0x100D), "0123456789ABCDEF"));
    EXPECT_TRUE(cursor.IsTruncated());
    EXPECT_EQ(cursor.GetWrittenLength(), 0);

    char afterTruncation[sizeof(buff)] = {};
    std::memcpy(afterTruncation, buff, sizeof(buff));

    //短字段本身放得下，但截断标志已经立起来，闩锁必须挡住它
    EXPECT_FALSE(cursor.AppendField("{}={:d}", static_cast<UInt16Type>(0x0001), 1));
    EXPECT_EQ(cursor.GetWrittenLength(), 0);
    EXPECT_EQ(std::string(buff, sizeof(buff)), std::string(afterTruncation, sizeof(afterTruncation)));
}

//WriteString(std::string) 是唯一一处线上字节与 sprintf 时代不同：{:s} 写整个 std::string，
//%s 停在首个内嵌 NUL。生成代码的字符串字段全是 char[] C 串，走不到这里；此用例把差异钉成有意的
TEST(StepUtilityTest, WriteString_StdStringEmbeddedNul)
{
    const std::string embedded("cu\0x", 4);

    ExpectStepField(0x100D, embedded, embedded);
}

// ============================================================
// HeadToStream / HeadFromStream 往返测试
// ============================================================

TEST(StepUtilityTest, HeadToStream_InsufficientBuffer)
{
    HeadField head = {};
    head.Version = ProtocolVersionValue;

    char buff[StepHeadLen - 1] = {};
    EXPECT_EQ(StepUtility::HeadToStream(&head, buff, StepHeadLen - 1), 0);
}

//入口闸门就是 StepHeadLen：容量恰好等于包头长度时必须成功，这是"包头已定长"的直接证据
TEST(StepUtilityTest, HeadToStream_ExactCapacitySucceeds)
{
    HeadField head = {};
    head.Magic = ProtocolMagicValue;
    head.Version = ProtocolVersionValue;
    head.PackageId = 0x00A1;
    head.BodyLen = 8;
    head.MsgSeqNum = 0xFFFFFFFFu;
    head.MessageChain = 1;

    char buff[StepHeadLen] = {};
    EXPECT_EQ(StepUtility::HeadToStream(&head, buff, StepHeadLen), static_cast<int>(StepHeadLen));
}

//把包头逐字节钉成黄金串。定宽之后每一位都是零填充大写十六进制，长度与取值无关，
//所以黄金串本身就是 62 字节：Version 写字面量让版本升级成为绊线，PackageId 取 0x00A1
//让大写与补零同时可见，MsgSeqNum 取 0xFFFFFFFF 钉住 8 位十六进制的上界
TEST(StepUtilityTest, HeadToStream_ByteExactGolden)
{
    HeadField head = {};
    head.Magic = ProtocolMagicValue;
    head.Version = ProtocolVersionValue;
    head.PackageId = 0x00A1;
    head.BodyLen = 8;
    head.MsgSeqNum = 0xFFFFFFFFu;
    head.MessageChain = 1;

    char buff[StepHeadLen] = {};
    int headLen = StepUtility::HeadToStream(&head, buff, StepHeadLen);

    std::string expected = std::string(1, SOH) + "0000=" + ProtocolMagicText + std::string(1, SOH)
                         + "0008=0003" + std::string(1, SOH)
                         + "0001=00A1" + std::string(1, SOH)
                         + "0002=0008" + std::string(1, SOH)
                         + "0004=FFFFFFFF" + std::string(1, SOH)
                         + "0003=1" + std::string(1, SOH);

    ASSERT_EQ(expected.size(), StepHeadLen);
    EXPECT_EQ(headLen, static_cast<int>(expected.size()));
    EXPECT_EQ(std::string(buff, buff + headLen), expected);

    //0xFFFFFFFF 在有符号下是 -1，能完成往返就说明补码歧义已经消除
    HeadField parsed = {};
    int headEndIndex = -1;
    EXPECT_TRUE(StepUtility::HeadFromStream(&buff[0], 0, headLen, &parsed, headEndIndex));
    EXPECT_EQ(parsed.MsgSeqNum, 0xFFFFFFFFu);
    EXPECT_EQ(parsed.PackageId, 0x00A1);
}

TEST(StepUtilityTest, HeadStreamRoundTrip)
{
    std::string stream = MakeStepHeadStream(0x1001, 128, 42, 0);

    int headLen = (int)stream.size();
    EXPECT_EQ(headLen, static_cast<int>(StepHeadLen));

    HeadField parsed = {};
    int headEndIndex = -1;
    EXPECT_TRUE(StepUtility::HeadFromStream(&stream[0], 0, headLen, &parsed, headEndIndex));
    EXPECT_EQ(headEndIndex, headLen);
    EXPECT_EQ(parsed.Magic, ProtocolMagicValue);
    EXPECT_EQ(parsed.Version, ProtocolVersionValue);
    EXPECT_EQ(parsed.PackageId, 0x1001);
    EXPECT_EQ(parsed.BodyLen, 128);
    EXPECT_EQ(parsed.MsgSeqNum, 42);
    EXPECT_EQ(parsed.MessageChain, 0);
}

TEST(StepUtilityTest, HeadStreamRoundTrip_MinValues)
{
    std::string stream = MakeStepHeadStream(0x0001, 0, 0, 0);

    int headLen = static_cast<int>(stream.size());
    EXPECT_EQ(headLen, static_cast<int>(StepHeadLen));

    HeadField parsed = {};
    int headEndIndex = -1;
    EXPECT_TRUE(StepUtility::HeadFromStream(&stream[0], 0, headLen, &parsed, headEndIndex));
    EXPECT_EQ(headEndIndex, headLen);
    EXPECT_EQ(parsed.PackageId, 0x0001);
    EXPECT_EQ(parsed.BodyLen, 0);
    EXPECT_EQ(parsed.MsgSeqNum, 0u);
    EXPECT_EQ(parsed.MessageChain, 0);
}

TEST(StepUtilityTest, HeadStreamRoundTrip_MaxValues)
{
    std::string stream = MakeStepHeadStream(0xFFFF, 65535, 0xFFFFFFFFu, 1);

    //每个字段都取上界，包头长度仍是 StepHeadLen：定宽之后长度与取值无关，
    //这正是 MakePackage 能一次写完包头的前提
    int headLen = static_cast<int>(stream.size());
    EXPECT_EQ(headLen, static_cast<int>(StepHeadLen));

    HeadField parsed = {};
    int headEndIndex = -1;
    EXPECT_TRUE(StepUtility::HeadFromStream(&stream[0], 0, headLen, &parsed, headEndIndex));
    EXPECT_EQ(parsed.PackageId, 0xFFFF);
    EXPECT_EQ(parsed.BodyLen, 65535);
    EXPECT_EQ(parsed.MsgSeqNum, 0xFFFFFFFFu);
    EXPECT_EQ(parsed.MessageChain, 1);
}

//0x80000000 在有符号下是负数，选它做往返是为了证明补码歧义确实消除
TEST(StepUtilityTest, HeadStreamRoundTrip_HighBitMessageChainSeqNum)
{
    std::string stream = MakeStepHeadStream(0x0001, 0, 0x80000000u, 0);

    HeadField parsed = {};
    int headEndIndex = -1;
    EXPECT_TRUE(StepUtility::HeadFromStream(&stream[0], 0, static_cast<int>(stream.size()), &parsed, headEndIndex));
    EXPECT_EQ(parsed.MsgSeqNum, 0x80000000u);
}

TEST(StepUtilityTest, HeadFromStream_BodyFollowsHead)
{
    // 包头后面接包体时，headEndIndex 必须正好停在包体起点
    std::string body = MakeStepField(0x100D, "600001");
    std::string stream = MakeStepHeadStream(0x1001, static_cast<UInt16Type>(body.size()), 7, 0) + body;

    HeadField parsed = {};
    int headEndIndex = -1;
    EXPECT_TRUE(StepUtility::HeadFromStream(&stream[0], 0, (int)stream.size(), &parsed, headEndIndex));
    EXPECT_EQ(headEndIndex, (int)(stream.size() - body.size()));
    EXPECT_EQ(parsed.BodyLen, static_cast<unsigned short>(body.size()));
}

TEST(StepUtilityTest, HeadFromStream_MissingKey)
{
    // 少一个字段（这里去掉 MessageChain）必须判失败，六个字段全到齐才算解析成功
    std::string stream = std::string(1, SOH)
                       + MakeStepField(Items::Magic, ProtocolMagicText)
                       + MakeStepField(Items::Version, kStepVersionText)
                       + MakeStepField(Items::PackageId, "1001")
                       + MakeStepField(Items::BodyLen, "0008")
                       + MakeStepField(Items::MsgSeqNum, "00000007")
                       + MakeStepField(0x100D, "600001");

    HeadField parsed = {};
    int headEndIndex = -1;
    EXPECT_FALSE(StepUtility::HeadFromStream(&stream[0], 0, (int)stream.size(), &parsed, headEndIndex));
}

TEST(StepUtilityTest, HeadFromStream_UnknownKey)
{
    // 首个字段就不是包头字段，等于一个字段都没解出来
    std::string stream = std::string(1, SOH) + MakeStepField(0xFFFF, "test");

    HeadField parsed = {};
    int headEndIndex = -1;
    EXPECT_FALSE(StepUtility::HeadFromStream(&stream[0], 0, (int)stream.size(), &parsed, headEndIndex));
}

TEST(StepUtilityTest, HeadFromStream_WrongMagic)
{
    std::string stream = MakeRawStepHeadStream("XPK2", kStepVersionText, "1001", "0008", "00000007", "0", MakeStepField(0x100D, "600001"));

    HeadField parsed = {};
    int headEndIndex = -1;
    EXPECT_FALSE(StepUtility::HeadFromStream(&stream[0], 0, (int)stream.size(), &parsed, headEndIndex));
}

TEST(StepUtilityTest, HeadFromStream_UnparseableValue)
{
    // 值含非十六进制字符必须判失败。旧实现用 std::stoi，抛出的异常会直接穿过线程入口终止进程
    std::string stream = MakeRawStepHeadStream(ProtocolMagicText, "000G", "1001", "0008", "00000007", "0", MakeStepField(0x100D, "600001"));

    HeadField parsed = {};
    int headEndIndex = -1;
    EXPECT_FALSE(StepUtility::HeadFromStream(&stream[0], 0, (int)stream.size(), &parsed, headEndIndex));
}

TEST(StepUtilityTest, HeadFromStream_TrailingGarbageValue)
{
    // "0008=2abcZ" 这种带尾巴的值也必须判失败，不能让解析器只吃掉前缀 "2abc" 就放过
    std::string stream = MakeRawStepHeadStream(ProtocolMagicText, "2abcZ", "1001", "0008", "00000007", "0", MakeStepField(0x100D, "600001"));

    HeadField parsed = {};
    int headEndIndex = -1;
    EXPECT_FALSE(StepUtility::HeadFromStream(&stream[0], 0, (int)stream.size(), &parsed, headEndIndex));
}

TEST(StepUtilityTest, HeadFromStream_BodyLenOutOfRange)
{
    // 包体长度是 16 位无符号，超范围必须判失败，不能截断成 0x9999 = 39321
    std::string stream = MakeRawStepHeadStream(ProtocolMagicText, kStepVersionText, "1001", "99999", "00000007", "0", MakeStepField(0x100D, "600001"));

    HeadField parsed = {};
    int headEndIndex = -1;
    EXPECT_FALSE(StepUtility::HeadFromStream(&stream[0], 0, static_cast<int>(stream.size()), &parsed, headEndIndex));
}

TEST(StepUtilityTest, HeadFromStream_NegativeMsgSeqNum)
{
    // MsgSeqNum 是无符号的，from_chars 对无符号目标拒绝 '-'，不得把 -1 读成补码大数
    std::string stream = MakeRawStepHeadStream(ProtocolMagicText, kStepVersionText, "1001", "0008", "-1", "0", MakeStepField(0x100D, "600001"));

    HeadField parsed = {};
    int headEndIndex = -1;
    EXPECT_FALSE(StepUtility::HeadFromStream(&stream[0], 0, static_cast<int>(stream.size()), &parsed, headEndIndex));
}

TEST(StepUtilityTest, HeadFromStream_EmptyValue)
{
    std::string stream = MakeRawStepHeadStream(ProtocolMagicText, kStepVersionText, "1001", "", "00000007", "0", MakeStepField(0x100D, "600001"));

    HeadField parsed = {};
    int headEndIndex = -1;
    EXPECT_FALSE(StepUtility::HeadFromStream(&stream[0], 0, (int)stream.size(), &parsed, headEndIndex));
}

// ============================================================
// TailToStream / TailFromStream 往返测试
// ============================================================

TEST(StepUtilityTest, TailToStream_InsufficientBuffer)
{
    TailField tail = {};
    tail.CheckSum = 123;

    char buff[StepTailLen] = {};
    EXPECT_EQ(StepUtility::TailToStream(&tail, buff, StepTailLen - 1), 0);
}

TEST(StepUtilityTest, TailStreamRoundTrip)
{
    std::string buff = MakeStepTailStream(123);

    // 报尾固定 "0005=" + 8 位十六进制 + SOH，共 14 字节
    ASSERT_EQ(buff.size(), StepTailLen);
    EXPECT_EQ(buff.substr(0, 5), "0005=");
    EXPECT_EQ(buff[StepTailLen - 1], SOH);

    TailField parsed = {};
    EXPECT_TRUE(StepUtility::TailFromStream(&buff[0], 0, (int)buff.size(), &parsed));
    EXPECT_EQ(parsed.CheckSum, 123u);
}

TEST(StepUtilityTest, TailStreamRoundTrip_Zero)
{
    std::string buff = MakeStepTailStream(0);

    TailField parsed = {};
    EXPECT_TRUE(StepUtility::TailFromStream(&buff[0], 0, (int)buff.size(), &parsed));
    EXPECT_EQ(parsed.CheckSum, 0u);
}

TEST(StepUtilityTest, TailStreamRoundTrip_Max)
{
    // 校验和是全 32 位：0xFFFFFFFF 在有符号下是 -1，能完成往返就说明补码歧义已消除
    std::string buff = MakeStepTailStream(0xFFFFFFFFu);

    TailField parsed = {};
    EXPECT_TRUE(StepUtility::TailFromStream(&buff[0], 0, (int)buff.size(), &parsed));
    EXPECT_EQ(parsed.CheckSum, 0xFFFFFFFFu);
}

TEST(StepUtilityTest, TailFromStream_InvalidKey)
{
    // TailFromStream 不会跳过前导字节，数据应为标准 key=value\SOH 格式
    std::string tailStream = MakeStepField(0xFFFF, "test");
    TailField tail = {};
    EXPECT_FALSE(StepUtility::TailFromStream(&tailStream[0], 0, (int)tailStream.size(), &tail));
}

TEST(StepUtilityTest, TailFromStream_OutOfRangeCheckSum)
{
    // 9 位十六进制超过 32 位，必须判失败而不是截断
    std::string tailStream = MakeStepField(Items::CheckSum, "1FFFFFFFF");
    TailField tail = {};
    EXPECT_FALSE(StepUtility::TailFromStream(&tailStream[0], 0, (int)tailStream.size(), &tail));
}

TEST(StepUtilityTest, TailFromStream_NoField)
{
    // 一个字段都没解析出来必须判失败，否则会被当成校验和为 0 的合法报尾
    char buff[1] = { 0 };
    TailField tail = {};
    EXPECT_FALSE(StepUtility::TailFromStream(buff, 0, 0, &tail));
}

// ============================================================
// 综合：构造一个完整 Head + Body + Tail 报文
// ============================================================

TEST(StepUtilityTest, CompleteHeadBodyTail)
{
    // 包体是纯字段数据流，不带前导 SOH
    std::string body = MakeStepField(0x100D, "600001") + MakeStepField(0x6015, "12.345");

    HeadField head = {};
    head.Magic = ProtocolMagicValue;
    head.Version = ProtocolVersionValue;
    head.PackageId = 0x1001;
    head.BodyLen = static_cast<UInt16Type>(body.size());
    head.MsgSeqNum = 1;
    head.MessageChain = 0;

    char headBuf[StepHeadLen] = {};
    int headLen = StepUtility::HeadToStream(&head, headBuf, StepHeadLen);
    ASSERT_EQ(headLen, static_cast<int>(StepHeadLen));

    std::string message(headBuf, headBuf + headLen);
    message += body;
    // 校验和覆盖包头与包体，与 Package::MakePackage 的算法一致
    auto checkSum = CalculateCrc32c(reinterpret_cast<const unsigned char*>(message.data()), (int)message.size());
    message += MakeStepTailStream(checkSum);

    // 反向解析：包头
    HeadField parsedHead = {};
    int headEndIndex = -1;
    EXPECT_TRUE(StepUtility::HeadFromStream(&message[0], 0, (int)message.size(), &parsedHead, headEndIndex));
    EXPECT_EQ(parsedHead.PackageId, 0x1001);
    EXPECT_EQ(parsedHead.MsgSeqNum, 1u);
    EXPECT_EQ(headEndIndex, headLen);
    EXPECT_EQ(message.substr(headLen, body.size()), body);

    // 反向解析：报尾
    int tailIndex = headEndIndex + parsedHead.BodyLen;
    ASSERT_EQ(static_cast<size_t>(tailIndex + StepTailLen), message.size());

    TailField parsedTail = {};
    EXPECT_TRUE(StepUtility::TailFromStream(&message[0], tailIndex, tailIndex + StepTailLen, &parsedTail));
    EXPECT_EQ(parsedTail.CheckSum, checkSum);
}

// ============================================================
// 边界：空/无效缓冲
// ============================================================

TEST(StepUtilityTest, EmptyBuffer_AllFunctionsReturnFalse)
{
    char buff[1] = { 0 };
    int sohIdx = -1, equalIdx = -1;
    unsigned short key = 0;
    std::string value;

    EXPECT_FALSE(StepUtility::GetNextSoh(buff, 0, 0, sohIdx));
    EXPECT_FALSE(StepUtility::GetNextEqual(buff, 0, 0, equalIdx));
}
