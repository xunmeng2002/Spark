#include <gtest/gtest.h>
#include <Spark/Network/Protocol/StepUtility.h>
#include <Spark/Network/Protocol/Head.h>
#include <Spark/Network/Protocol/Items.h>

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
    std::string MakeStepField(unsigned short key, const std::string& value)
    {
        return std::to_string(key) + "=" + value + std::string(1, kSOH);
    }

    std::string MakeStepField(int key, const std::string& value)
    {
        return std::to_string(key) + "=" + value + std::string(1, kSOH);
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
// GetPackageStart（需要前导 SOH 后跟 "1="）
// ============================================================

TEST(StepUtilityTest, GetPackageStart_Found)
{
    // SOH + "1=" is the package start marker
    std::string data = std::string("skip") + std::string(1, kSOH) + MakeStepField(1, "0001");
    int startIdx = -1;

    EXPECT_TRUE(StepUtility::GetPackageStart(&data[0], 0, (int)data.size(), startIdx));
    EXPECT_EQ(startIdx, 4);
}

TEST(StepUtilityTest, GetPackageStart_AtBeginning)
{
    std::string data = std::string(1, kSOH) + MakeStepField(1, "0001");
    int startIdx = -1;

    EXPECT_TRUE(StepUtility::GetPackageStart(&data[0], 0, (int)data.size(), startIdx));
    EXPECT_EQ(startIdx, 0);
}

TEST(StepUtilityTest, GetPackageStart_NotFound)
{
    std::string data = std::string(1, kSOH) + MakeStepField(2, "0005");
    int startIdx = -1;

    EXPECT_FALSE(StepUtility::GetPackageStart(&data[0], 0, (int)data.size(), startIdx));
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

TEST(StepUtilityTest, HeadStreamRoundTrip)
{
    HeadField head = {};
    head.PackageID = 0x1001;
    head.BodyLen = 128;
    head.MsgSeqNum = 42;
    head.MessageChain = 0;

    char buff[StepHeaderLen + 1] = {};
    StepUtility::HeadToStream(&head, buff, StepHeaderLen);

    // 验证头流长度
    int headLen = (int)strlen(buff);
    EXPECT_GT(headLen, 0);
    EXPECT_LE(headLen, StepHeaderLen);

    // 反向解析
    HeadField parsed = {};
    bool ok = StepUtility::HeadFromStream(buff, 0, StepHeaderLen, &parsed);
    EXPECT_TRUE(ok);
    EXPECT_EQ(parsed.PackageID, head.PackageID);
    EXPECT_EQ(parsed.BodyLen, head.BodyLen);
    EXPECT_EQ(parsed.MsgSeqNum, head.MsgSeqNum);
    EXPECT_EQ(parsed.MessageChain, head.MessageChain);
}

TEST(StepUtilityTest, HeadStreamRoundTrip_MinValues)
{
    HeadField head = {};
    head.PackageID = 0x0001;
    head.BodyLen = 0;
    head.MsgSeqNum = 0;
    head.MessageChain = 0;

    char buff[StepHeaderLen + 1] = {};
    StepUtility::HeadToStream(&head, buff, StepHeaderLen);

    HeadField parsed = {};
    EXPECT_TRUE(StepUtility::HeadFromStream(buff, 0, StepHeaderLen, &parsed));
    EXPECT_EQ(parsed.PackageID, 0x0001);
    EXPECT_EQ(parsed.BodyLen, 0);
    EXPECT_EQ(parsed.MsgSeqNum, 0);
    EXPECT_EQ(parsed.MessageChain, 0);
}

TEST(StepUtilityTest, HeadStreamRoundTrip_MaxValues)
{
    HeadField head = {};
    head.PackageID = 0xFFFF;
    head.BodyLen = 65535;       // UShortType max
    head.MsgSeqNum = 999999999;
    head.MessageChain = 1;

    char buff[StepHeaderLen + 1] = {};
    StepUtility::HeadToStream(&head, buff, StepHeaderLen);

    HeadField parsed = {};
    EXPECT_TRUE(StepUtility::HeadFromStream(buff, 0, StepHeaderLen, &parsed));
    EXPECT_EQ(parsed.PackageID, 0xFFFF);
    EXPECT_EQ(parsed.BodyLen, 65535);
    EXPECT_EQ(parsed.MsgSeqNum, 999999999);
    EXPECT_EQ(parsed.MessageChain, 1);
}

TEST(StepUtilityTest, HeadFromStream_InvalidKey)
{
    // 构造一个包含未知 key 的头流
    // HeadFromStream 跳过首个 SOH，数据必须以前导 SOH 开头
    std::string headStream = std::string(1, kSOH) + MakeStepField(0xFFFF, "test");
    HeadField head = {};
    EXPECT_FALSE(StepUtility::HeadFromStream(&headStream[0], 0, (int)headStream.size(), &head));
}

// ============================================================
// TailToStream / TailFromStream 往返测试
// ============================================================

TEST(StepUtilityTest, TailStreamRoundTrip)
{
    TailField tail = {};
    tail.CheckSum = 123;

    char buff[StepTailLen + 1] = {};
    StepUtility::TailToStream(&tail, buff, StepTailLen);

    TailField parsed = {};
    EXPECT_TRUE(StepUtility::TailFromStream(buff, 0, StepTailLen, &parsed));
    EXPECT_EQ(parsed.CheckSum, 123);
}

TEST(StepUtilityTest, TailStreamRoundTrip_Zero)
{
    TailField tail = {};
    tail.CheckSum = 0;

    char buff[StepTailLen + 1] = {};
    StepUtility::TailToStream(&tail, buff, StepTailLen);

    TailField parsed = {};
    EXPECT_TRUE(StepUtility::TailFromStream(buff, 0, StepTailLen, &parsed));
    EXPECT_EQ(parsed.CheckSum, 0);
}

TEST(StepUtilityTest, TailStreamRoundTrip_Max)
{
    TailField tail = {};
    tail.CheckSum = 255;

    char buff[StepTailLen + 1] = {};
    StepUtility::TailToStream(&tail, buff, StepTailLen);

    TailField parsed = {};
    EXPECT_TRUE(StepUtility::TailFromStream(buff, 0, StepTailLen, &parsed));
    EXPECT_EQ(parsed.CheckSum, 255);
}

TEST(StepUtilityTest, TailFromStream_InvalidKey)
{
    // TailFromStream 不会跳过前导字节，数据应为标准 key=value\SOH 格式
    std::string tailStream = MakeStepField(0xFFFF, "test");
    TailField tail = {};
    EXPECT_FALSE(StepUtility::TailFromStream(&tailStream[0], 0, (int)tailStream.size(), &tail));
}

// ============================================================
// 综合：构造一个完整 Head + Body + Tail 报文
// ============================================================

TEST(StepUtilityTest, CompleteHeadBodyTail)
{
    HeadField head = {};
    head.PackageID = 0x1001;
    head.BodyLen = 8;
    head.MsgSeqNum = 1;
    head.MessageChain = 0;

    char headBuf[StepHeaderLen + 1] = {};
    StepUtility::HeadToStream(&head, headBuf, StepHeaderLen);

    // 构造 body: 简单字段（无前导 SOH，body 是纯字段数据流）
    std::string body = MakeStepField(0x100D, "600001") + MakeStepField(0x6015, "12.345");

    // 计算 body 校验和
    TailField tail = {};
    tail.CheckSum = 42;

    char tailBuf[StepTailLen + 1] = {};
    StepUtility::TailToStream(&tail, tailBuf, StepTailLen);

    // 组装完整报文
    std::string message = std::string(headBuf) + body + tailBuf;
    EXPECT_GT(message.size(), (size_t)(StepHeaderLen + StepTailLen));

    // 反向解析 head
    HeadField parsedHead = {};
    EXPECT_TRUE(StepUtility::HeadFromStream(&message[0], 0, StepHeaderLen, &parsedHead));
    EXPECT_EQ(parsedHead.PackageID, 0x1001);
    EXPECT_EQ(parsedHead.MsgSeqNum, 1);
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
    EXPECT_FALSE(StepUtility::GetPackageStart(buff, 0, 0, sohIdx));
}
