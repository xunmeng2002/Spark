#include <gtest/gtest.h>
#include <Spark/Network/Protocol/StepUtility.h>
#include <Spark/Network/Protocol/Head.h>
#include <Spark/Network/Protocol/Items.h>
#include <Spark/Network/Protocol/ProtocolUtility.h>
#include <Spark/Network/Protocol/ProtocolVersion.h>

#include <cstring>
#include <string>
using namespace spark::network;
// ============================================================
// StepUtility 测试
// STEP 协议缓冲解析/序列化：字段定位、报文头/尾流式转换
// ============================================================

namespace
{
    constexpr char kSOH = 1;

    // 构造 "key=value\SOH"（无前导 SOH，用于 GetNext 直接解析）
    // 只留一个重载：Items 里的字段 ID 是 unsigned int，另外两个重载会让调用变成二义
    std::string MakeStepField(unsigned int key, const std::string& value)
    {
        return std::to_string(key) + "=" + value + std::string(1, kSOH);
    }

    // 按 HeadToStream 的写法造一个完整包头，返回实际写出的字节
    std::string MakeStepHeadStream(unsigned short packageID, unsigned short bodyLen, int msgSeqNum, int messageChain)
    {
        HeadField head = {};
        head.Magic = ProtocolMagicValue;
        head.Version = ProtocolVersionValue;
        head.PackageID = packageID;
        head.BodyLen = bodyLen;
        head.MsgSeqNum = msgSeqNum;
        head.MessageChain = messageChain;

        char buff[StepMaxHeaderLen] = {};
        int len = StepUtility::HeadToStream(&head, buff, StepMaxHeaderLen);
        return std::string(buff, buff + len);
    }

    // 手工拼包头，只留出字段值可控的口子，便于构造坏值用例
    std::string MakeRawStepHeadStream(
        const std::string& magic = ProtocolMagicText,
        const std::string& version = std::to_string(ProtocolVersionValue),
        const std::string& packageID = "1001",
        const std::string& bodyLen = "00008",
        const std::string& msgSeqNum = "7",
        const std::string& messageChain = "0",
        const std::string& trailing = "")
    {
        return std::string(1, kSOH)
            + MakeStepField(Items::Magic, magic)
            + MakeStepField(Items::Version, version)
            + MakeStepField(Items::PackageID, packageID)
            + MakeStepField(Items::BodyLen, bodyLen)
            + MakeStepField(Items::MsgSeqNum, msgSeqNum)
            + MakeStepField(Items::MessageChain, messageChain)
            + trailing;
    }

    // 按 TailToStream 的写法造一个完整报尾，返回实际写出的字节
    std::string MakeStepTailStream(unsigned int checkSum)
    {
        TailField tail = {};
        tail.CheckSum = static_cast<IntType>(checkSum);

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
    char buff[] = { 'a', 'b', kSOH, 'c', 'd' };
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
    char buff[] = { kSOH, 'a' };
    int sohIndex = -1;
    EXPECT_FALSE(StepUtility::GetNextSoh(buff, 2, 2, sohIndex));
}

// ============================================================
// GetNextEqual
// ============================================================

TEST(StepUtilityTest, GetNextEqual_Found)
{
    char buff[] = { '1', '=', '2', kSOH };
    int equalIndex = -1;
    EXPECT_TRUE(StepUtility::GetNextEqual(buff, 0, 4, equalIndex));
    EXPECT_EQ(equalIndex, 1);
}

TEST(StepUtilityTest, GetNextEqual_NotFound)
{
    char buff[] = { '1', '2', '3', kSOH };
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
    char buff[] = { '1', 'a', 'b', kSOH };
    unsigned short key = 0;
    std::string value;
    int sohIndex = -1;

    EXPECT_FALSE(StepUtility::GetNext(buff, 0, 4, key, value, sohIndex));
}

// ============================================================
// 报文起始锚点（SOH + "0=SPK2" + SOH）
// ============================================================

TEST(StepUtilityTest, PackageStartAnchor_Format)
{
    const std::string& anchor = StepUtility::GetPackageStartAnchor();
    // SOH + "0" + "=" + "SPK2" + SOH，共 8 字节
    ASSERT_EQ(anchor.size(), 8u);
    EXPECT_EQ(anchor[0], kSOH);
    EXPECT_EQ(anchor[1], '0');
    EXPECT_EQ(anchor[2], '=');
    EXPECT_EQ(anchor.substr(3, 4), "SPK2");
    EXPECT_EQ(anchor[7], kSOH);
}

TEST(StepUtilityTest, PackageStartAnchor_MatchesMagicValueBytes)
{
    // 锚点里的 "SPK2" 必须与 ProtocolMagicValue 是小端的同一串字节
    const std::string& anchor = StepUtility::GetPackageStartAnchor();
    unsigned int magic = 0;
    memcpy(&magic, anchor.data() + 3, 4);
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
//（需要前导 SOH 后跟 "6=" / "7="）
// ============================================================

TEST(StepUtilityTest, GetFieldStart_Found)
{
    // FieldStart marker: SOH + "6=" + hexFieldID
    // 6=100D indicates field 0x100D
    std::string data = std::string(1, kSOH) + MakeStepField(6, "100D")
                     + "some_data"
                     + std::string(1, kSOH) + MakeStepField(7, "100D");
    unsigned short fieldID = 0;
    int startIndex = -1;

    EXPECT_TRUE(StepUtility::GetFieldStart(&data[0], 0, (int)data.size(), fieldID, startIndex));
    EXPECT_EQ(fieldID, 0x100D);
    EXPECT_GE(startIndex, 0);
}

TEST(StepUtilityTest, GetFieldStart_NotFound)
{
    std::string data = std::string(1, kSOH) + MakeStepField(2, "0005");  // no field start marker
    unsigned short fieldID = 0;
    int startIndex = -1;

    EXPECT_FALSE(StepUtility::GetFieldStart(&data[0], 0, (int)data.size(), fieldID, startIndex));
}

TEST(StepUtilityTest, GetFieldEnd_Found)
{
    std::string data = std::string(1, kSOH) + MakeStepField(7, "100D");
    unsigned short fieldID = 0;
    int endIndex = -1;

    EXPECT_TRUE(StepUtility::GetFieldEnd(&data[0], 0, (int)data.size(), fieldID, endIndex));
    EXPECT_EQ(fieldID, 0x100D);
}

TEST(StepUtilityTest, GetNextFieldZone_Complete)
{
    std::string data = std::string(1, kSOH) + MakeStepField(6, "100D")
                     + "content"
                     + std::string(1, kSOH) + MakeStepField(7, "100D");
    unsigned short fieldID = 0;
    int startIdx = -1, endIdx = -1;

    EXPECT_TRUE(StepUtility::GetNextFieldZone(&data[0], 0, (int)data.size(), fieldID, startIdx, endIdx));
    EXPECT_EQ(fieldID, 0x100D);
    EXPECT_GE(startIdx, 0);
    EXPECT_GT(endIdx, startIdx);
}

TEST(StepUtilityTest, GetNextFieldZone_MismatchedIDs)
{
    // FieldID 和 FieldEnd 的 ID 不匹配
    std::string data = std::string(1, kSOH) + MakeStepField(6, "100D")
                     + "content"
                     + std::string(1, kSOH) + MakeStepField(7, "100E");
    unsigned short fieldID = 0;
    int startIdx = -1, endIdx = -1;

    EXPECT_FALSE(StepUtility::GetNextFieldZone(&data[0], 0, (int)data.size(), fieldID, startIdx, endIdx));
}

// ============================================================
// WriteString（所有重载）
// ============================================================

TEST(StepUtilityTest, WriteString_Bool)
{
    char buff[64] = {};
    char* ppos = buff;
    StepUtility::WriteString(ppos, 0x8001, true);   // IsAllowLogin
    *ppos = '\0';

    EXPECT_GT(strlen(buff), 0);
    // Verify the format: key=value\SOH
    EXPECT_EQ(buff[strlen(buff) - 1], kSOH);
}

TEST(StepUtilityTest, WriteString_Char)
{
    char buff[64] = {};
    char* ppos = buff;
    StepUtility::WriteString(ppos, 0x9005, 'B');    // Direction
    *ppos = '\0';

    EXPECT_GT(strlen(buff), 0);
    EXPECT_EQ(buff[strlen(buff) - 1], kSOH);
}

TEST(StepUtilityTest, WriteString_UnsignedShort)
{
    char buff[64] = {};
    char* ppos = buff;
    StepUtility::WriteString(ppos, 0x0001, (unsigned short)0x0001);  // PackageID
    *ppos = '\0';

    EXPECT_GT(strlen(buff), 0);
}

TEST(StepUtilityTest, WriteString_Int)
{
    char buff[64] = {};
    char* ppos = buff;
    StepUtility::WriteString(ppos, 0x0004, 123456);  // MsgSeqNum
    *ppos = '\0';

    EXPECT_GT(strlen(buff), 0);
}

TEST(StepUtilityTest, WriteString_LongLong)
{
    char buff[64] = {};
    char* ppos = buff;
    StepUtility::WriteString(ppos, 0x3001, 20240115LL);  // TradingDay
    *ppos = '\0';

    std::string result(buff);
    EXPECT_GT(result.size(), 0);
    EXPECT_EQ(result[result.size() - 1], kSOH);
}

TEST(StepUtilityTest, WriteString_Double)
{
    char buff[64] = {};
    char* ppos = buff;
    StepUtility::WriteString(ppos, 0x6015, 12.345);  // Price
    *ppos = '\0';

    std::string result(buff);
    EXPECT_GT(result.size(), 0);
    EXPECT_EQ(result[result.size() - 1], kSOH);
    // 默认精度 6 位小数
    EXPECT_NE(result.find(".345"), std::string::npos) << result;
}

TEST(StepUtilityTest, WriteString_StdString)
{
    char buff[64] = {};
    char* ppos = buff;
    StepUtility::WriteString(ppos, 0x100D, std::string("600001"));  // InstrumentID
    *ppos = '\0';

    EXPECT_GT(strlen(buff), 0);
}

TEST(StepUtilityTest, WriteString_CharPtr)
{
    char buff[64] = {};
    char* ppos = buff;
    StepUtility::WriteString(ppos, 0x100D, "cu2401");
    *ppos = '\0';

    EXPECT_GT(strlen(buff), 0);
}

TEST(StepUtilityTest, WriteHexString)
{
    char buff[64] = {};
    char* ppos = buff;
    StepUtility::WriteHexString(ppos, 0x0001, 0x00FF);  // test with value 255
    *ppos = '\0';

    std::string result(buff);
    EXPECT_GT(result.size(), 0);
    // hex format: key=00FF\SOH
    EXPECT_NE(result.find("00FF"), std::string::npos) << result;
    EXPECT_EQ(result[result.size() - 1], kSOH);
}

// ============================================================
// HeadToStream / HeadFromStream 往返测试
// ============================================================

TEST(StepUtilityTest, HeadToStream_InsufficientBuffer)
{
    HeadField head = {};
    head.Version = ProtocolVersionValue;

    char buff[StepMaxHeaderLen - 1] = {};
    EXPECT_EQ(StepUtility::HeadToStream(&head, buff, StepMaxHeaderLen - 1), 0);
}

TEST(StepUtilityTest, HeadStreamRoundTrip)
{
    std::string stream = MakeStepHeadStream(0x1001, 128, 42, 0);

    int headLen = (int)stream.size();
    EXPECT_GT(headLen, 0);
    EXPECT_LT(headLen, (int)StepMaxHeaderLen);

    HeadField parsed = {};
    int headEndIndex = -1;
    EXPECT_TRUE(StepUtility::HeadFromStream(&stream[0], 0, headLen, &parsed, headEndIndex));
    EXPECT_EQ(headEndIndex, headLen);
    EXPECT_EQ(parsed.Magic, ProtocolMagicValue);
    EXPECT_EQ(parsed.Version, ProtocolVersionValue);
    EXPECT_EQ(parsed.PackageID, 0x1001);
    EXPECT_EQ(parsed.BodyLen, 128);
    EXPECT_EQ(parsed.MsgSeqNum, 42);
    EXPECT_EQ(parsed.MessageChain, 0);
}

TEST(StepUtilityTest, HeadStreamRoundTrip_MinValues)
{
    std::string stream = MakeStepHeadStream(0x0001, 0, 0, 0);

    HeadField parsed = {};
    int headEndIndex = -1;
    EXPECT_TRUE(StepUtility::HeadFromStream(&stream[0], 0, (int)stream.size(), &parsed, headEndIndex));
    EXPECT_EQ(headEndIndex, (int)stream.size());
    EXPECT_EQ(parsed.PackageID, 0x0001);
    EXPECT_EQ(parsed.BodyLen, 0);
    EXPECT_EQ(parsed.MsgSeqNum, 0);
    EXPECT_EQ(parsed.MessageChain, 0);
}

TEST(StepUtilityTest, HeadStreamRoundTrip_MaxValues)
{
    std::string stream = MakeStepHeadStream(0xFFFF, 65535, 999999999, 1);

    HeadField parsed = {};
    int headEndIndex = -1;
    EXPECT_TRUE(StepUtility::HeadFromStream(&stream[0], 0, (int)stream.size(), &parsed, headEndIndex));
    EXPECT_EQ(parsed.PackageID, 0xFFFF);
    EXPECT_EQ(parsed.BodyLen, 65535);
    EXPECT_EQ(parsed.MsgSeqNum, 999999999);
    EXPECT_EQ(parsed.MessageChain, 1);
}

TEST(StepUtilityTest, HeadFromStream_BodyFollowsHead)
{
    // 包头后面接包体时，headEndIndex 必须正好停在包体起点
    std::string body = MakeStepField(0x100D, "600001");
    std::string stream = MakeStepHeadStream(0x1001, (unsigned short)body.size(), 7, 0) + body;

    HeadField parsed = {};
    int headEndIndex = -1;
    EXPECT_TRUE(StepUtility::HeadFromStream(&stream[0], 0, (int)stream.size(), &parsed, headEndIndex));
    EXPECT_EQ(headEndIndex, (int)(stream.size() - body.size()));
    EXPECT_EQ(parsed.BodyLen, static_cast<unsigned short>(body.size()));
}

TEST(StepUtilityTest, HeadFromStream_MissingKey)
{
    // 少一个字段（这里去掉 MessageChain）必须判失败，六个字段全到齐才算解析成功
    std::string stream = std::string(1, kSOH)
                       + MakeStepField(Items::Magic, ProtocolMagicText)
                       + MakeStepField(Items::Version, "2")
                       + MakeStepField(Items::PackageID, "1001")
                       + MakeStepField(Items::BodyLen, "00008")
                       + MakeStepField(Items::MsgSeqNum, "7")
                       + MakeStepField(0x100D, "600001");

    HeadField parsed = {};
    int headEndIndex = -1;
    EXPECT_FALSE(StepUtility::HeadFromStream(&stream[0], 0, (int)stream.size(), &parsed, headEndIndex));
}

TEST(StepUtilityTest, HeadFromStream_UnknownKey)
{
    // 首个字段就不是包头字段，等于一个字段都没解出来
    std::string stream = std::string(1, kSOH) + MakeStepField(0xFFFF, "test");

    HeadField parsed = {};
    int headEndIndex = -1;
    EXPECT_FALSE(StepUtility::HeadFromStream(&stream[0], 0, (int)stream.size(), &parsed, headEndIndex));
}

TEST(StepUtilityTest, HeadFromStream_WrongMagic)
{
    std::string stream = MakeRawStepHeadStream("XPK2", "2", "1001", "00008", "7", "0", MakeStepField(0x100D, "600001"));

    HeadField parsed = {};
    int headEndIndex = -1;
    EXPECT_FALSE(StepUtility::HeadFromStream(&stream[0], 0, (int)stream.size(), &parsed, headEndIndex));
}

TEST(StepUtilityTest, HeadFromStream_UnparseableValue)
{
    // 值不是数字必须判失败。旧实现用 std::stoi，抛出的异常会直接穿过线程入口终止进程
    std::string stream = MakeRawStepHeadStream(ProtocolMagicText, "abc", "1001", "00008", "7", "0", MakeStepField(0x100D, "600001"));

    HeadField parsed = {};
    int headEndIndex = -1;
    EXPECT_FALSE(StepUtility::HeadFromStream(&stream[0], 0, (int)stream.size(), &parsed, headEndIndex));
}

TEST(StepUtilityTest, HeadFromStream_TrailingGarbageValue)
{
    // "8=2abc" 这种带尾巴的值也必须判失败，不能让 stoi 只吃掉前缀就放过
    std::string stream = MakeRawStepHeadStream(ProtocolMagicText, "2abc", "1001", "00008", "7", "0", MakeStepField(0x100D, "600001"));

    HeadField parsed = {};
    int headEndIndex = -1;
    EXPECT_FALSE(StepUtility::HeadFromStream(&stream[0], 0, (int)stream.size(), &parsed, headEndIndex));
}

TEST(StepUtilityTest, HeadFromStream_BodyLenOutOfRange)
{
    // 包体长度是 16 位无符号，超范围必须判失败，不能截断成 34464
    std::string stream = MakeRawStepHeadStream(ProtocolMagicText, "2", "1001", "99999", "7", "0", MakeStepField(0x100D, "600001"));

    HeadField parsed = {};
    int headEndIndex = -1;
    EXPECT_FALSE(StepUtility::HeadFromStream(&stream[0], 0, (int)stream.size(), &parsed, headEndIndex));
}

TEST(StepUtilityTest, HeadFromStream_EmptyValue)
{
    std::string stream = MakeRawStepHeadStream(ProtocolMagicText, "2", "1001", "", "7", "0", MakeStepField(0x100D, "600001"));

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

    // 校验和扩到 32 位后，报尾固定 "5=" + 8 位十六进制 + SOH，共 11 字节
    ASSERT_EQ(buff.size(), StepTailLen);
    EXPECT_EQ(buff[0], '5');
    EXPECT_EQ(buff[1], '=');
    EXPECT_EQ(buff[StepTailLen - 1], kSOH);

    TailField parsed = {};
    EXPECT_TRUE(StepUtility::TailFromStream(&buff[0], 0, (int)buff.size(), &parsed));
    EXPECT_EQ(parsed.CheckSum, 123);
}

TEST(StepUtilityTest, TailStreamRoundTrip_Zero)
{
    std::string buff = MakeStepTailStream(0);

    TailField parsed = {};
    EXPECT_TRUE(StepUtility::TailFromStream(&buff[0], 0, (int)buff.size(), &parsed));
    EXPECT_EQ(parsed.CheckSum, 0);
}

TEST(StepUtilityTest, TailStreamRoundTrip_Max)
{
    // 旧实现是 unsigned short 的 4 位十六进制，最大只到 0xFFFF，这里钉住全 32 位
    std::string buff = MakeStepTailStream(0xFFFFFFFFu);

    TailField parsed = {};
    EXPECT_TRUE(StepUtility::TailFromStream(&buff[0], 0, (int)buff.size(), &parsed));
    EXPECT_EQ(static_cast<unsigned int>(parsed.CheckSum), 0xFFFFFFFFu);
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
    head.PackageID = 0x1001;
    head.BodyLen = static_cast<UShortType>(body.size());
    head.MsgSeqNum = 1;
    head.MessageChain = 0;

    char headBuf[StepMaxHeaderLen] = {};
    int headLen = StepUtility::HeadToStream(&head, headBuf, StepMaxHeaderLen);
    ASSERT_GT(headLen, 0);

    std::string message(headBuf, headBuf + headLen);
    message += body;
    // 校验和覆盖包头与包体，与 Package::MakePackage 的算法一致
    auto checkSum = CalculateCrc32c(reinterpret_cast<const unsigned char*>(message.data()), (int)message.size());
    message += MakeStepTailStream(checkSum);

    // 反向解析：包头
    HeadField parsedHead = {};
    int headEndIndex = -1;
    EXPECT_TRUE(StepUtility::HeadFromStream(&message[0], 0, (int)message.size(), &parsedHead, headEndIndex));
    EXPECT_EQ(parsedHead.PackageID, 0x1001);
    EXPECT_EQ(parsedHead.MsgSeqNum, 1);
    EXPECT_EQ(headEndIndex, headLen);
    EXPECT_EQ(message.substr(headLen, body.size()), body);

    // 反向解析：报尾
    int tailIndex = headEndIndex + parsedHead.BodyLen;
    ASSERT_EQ(static_cast<size_t>(tailIndex + StepTailLen), message.size());

    TailField parsedTail = {};
    EXPECT_TRUE(StepUtility::TailFromStream(&message[0], tailIndex, tailIndex + StepTailLen, &parsedTail));
    EXPECT_EQ(static_cast<unsigned int>(parsedTail.CheckSum), checkSum);
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
