#include <gtest/gtest.h>
#include <Spark/Network/Protocol/ProtocolUtility.h>
using namespace spark::network;
// ============================================================
// CalculateSum 测试
// 校验和计算：所有字节和 % 256
// ============================================================

TEST(CalculateSumTest, EmptyBuffer)
{
    unsigned char buff[1] = { 0 };
    EXPECT_EQ(CalculateSum(buff, 0), 0);
}

TEST(CalculateSumTest, SingleByte)
{
    unsigned char buff[] = { 0x41 };  // 'A'
    EXPECT_EQ(CalculateSum(buff, 1), 0x41);
}

TEST(CalculateSumTest, MultipleBytes)
{
    unsigned char buff[] = { 0x01, 0x02, 0x03, 0x04 };
    // 1 + 2 + 3 + 4 = 10
    EXPECT_EQ(CalculateSum(buff, 4), 10);
}

TEST(CalculateSumTest, SumExceeds256)
{
    // sum = 255 + 1 = 256 → 256 % 256 = 0
    unsigned char buff[] = { 0xFF, 0x01 };
    EXPECT_EQ(CalculateSum(buff, 2), 0);
}

TEST(CalculateSumTest, SumMultipleOf256)
{
    // 128 + 128 = 256 → 0
    unsigned char buff[] = { 128, 128 };
    EXPECT_EQ(CalculateSum(buff, 2), 0);
}

TEST(CalculateSumTest, LargeBuffer)
{
    unsigned char buff[100];
    memset(buff, 1, sizeof(buff));
    // 100 * 1 = 100 → 100 % 256 = 100
    EXPECT_EQ(CalculateSum(buff, 100), 100);
}

TEST(CalculateSumTest, AllMaxValues)
{
    unsigned char buff[] = { 0xFF, 0xFF };
    // 255 + 255 = 510 → 510 % 256 = 254
    EXPECT_EQ(CalculateSum(buff, 2), 254);
}

TEST(CalculateSumTest, SpecificPattern)
{
    // STEP 报文典型数据
    unsigned char buff[] = { 0x01, 0x31, 0x3D, 0x30, 0x30, 0x30, 0x31, 0x01 };
    // 1 + 49 + 61 + 48 + 48 + 48 + 49 + 1 = 305 → 305 % 256 = 49
    EXPECT_EQ(CalculateSum(buff, 8), 49);
}

// ============================================================
// CalculateCrc32c 测试
// 校验和计算：CRC-32C（Castagnoli），反射多项式 0x82F63B78
// 期望值由独立的 Python 参考实现算出，本文件只固化结果
// ============================================================

TEST(CalculateCrc32cTest, EmptyBuffer)
{
    unsigned char buff[1] = { 0 };
    // 初值与终值异或相互抵消，空输入必为 0
    EXPECT_EQ(CalculateCrc32c(buff, 0), 0x00000000u);
}

TEST(CalculateCrc32cTest, StandardCheckValue)
{
    // CRC-32C 的标准校验值，可同时钉住多项式、初值、反射方向与终值异或
    const unsigned char* buff = (const unsigned char*)"123456789";
    EXPECT_EQ(CalculateCrc32c(buff, 9), 0xE3069283u);
}

TEST(CalculateCrc32cTest, SingleByte)
{
    unsigned char buff[] = { 0x41 };  // 'A'
    EXPECT_EQ(CalculateCrc32c(buff, 1), 0xE16DCDEEu);
}

TEST(CalculateCrc32cTest, StepTypicalPattern)
{
    // STEP 报文典型数据，与 CalculateSumTest.SpecificPattern 同一组输入
    unsigned char buff[] = { 0x01, 0x31, 0x3D, 0x30, 0x30, 0x30, 0x31, 0x01 };
    EXPECT_EQ(CalculateCrc32c(buff, 8), 0xEE8606E7u);
}

TEST(CalculateCrc32cTest, SingleZeroByteIsNotEmpty)
{
    unsigned char buff[] = { 0x00 };
    EXPECT_EQ(CalculateCrc32c(buff, 1), 0x527D5351u);
}

TEST(CalculateCrc32cTest, LeadingZeroByteIsNotSkipped)
{
    // 前导 0x00 与"没有这个字节"不是一回事，跳字节的实现会算成 0xA0E9D052
    unsigned char buff[] = { 0x00, 0xFF, 0x01 };
    EXPECT_EQ(CalculateCrc32c(buff, 3), 0x31EC04FAu);
    EXPECT_NE(CalculateCrc32c(buff, 3), CalculateCrc32c(buff + 1, 2));
}

TEST(CalculateCrc32cTest, LargeBuffer)
{
    unsigned char buff[100];
    memset(buff, 1, sizeof(buff));
    EXPECT_EQ(CalculateCrc32c(buff, 100), 0xA8DB2B67u);
}

TEST(CalculateCrc32cTest, SingleBitFlipIsDetected)
{
    unsigned char buff[100];
    memset(buff, 1, sizeof(buff));
    auto origin = CalculateCrc32c(buff, 100);
    buff[57] ^= 0x80;
    EXPECT_NE(CalculateCrc32c(buff, 100), origin);
}

TEST(CalculateCrc32cTest, OnlyFirstLengthBytesCounted)
{
    // len 之外的字节不得参与计算，否则校验会覆盖到下一条报文的头
    unsigned char buff[] = { 0xFF, 0x01, 0x80, 0x80 };
    EXPECT_EQ(CalculateCrc32c(buff, 2), 0xA0E9D052u);
}

// ============================================================
// FindBytes 测试
// 报文重同步与报尾定位共用的逐字节模式查找
// ============================================================

TEST(FindBytesTest, FoundAtBeginning)
{
    const char data[] = { 'S', 'P', 'K', '2', 'x' };
    unsigned int offset = 99;
    EXPECT_TRUE(FindBytes(data, 5, "SPK2", 4, offset));
    EXPECT_EQ(offset, 0u);
}

TEST(FindBytesTest, FoundInMiddle)
{
    const char data[] = { 'a', 'b', 'c', 'S', 'P', 'K', '2' };
    unsigned int offset = 99;
    EXPECT_TRUE(FindBytes(data, 7, "SPK2", 4, offset));
    EXPECT_EQ(offset, 3u);
}

TEST(FindBytesTest, NotFound)
{
    const char data[] = { 'a', 'b', 'c', 'd' };
    unsigned int offset = 99;
    EXPECT_FALSE(FindBytes(data, 4, "SPK2", 4, offset));
    EXPECT_EQ(offset, 99u);  // 未命中时不得写出参
}

TEST(FindBytesTest, PatternLongerThanData)
{
    const char data[] = { 'S', 'P', 'K' };
    unsigned int offset = 0;
    EXPECT_FALSE(FindBytes(data, 3, "SPK2", 4, offset));
}

TEST(FindBytesTest, EmptyPatternOrEmptyData)
{
    const char data[] = { 'S', 'P', 'K', '2' };
    unsigned int offset = 0;
    EXPECT_FALSE(FindBytes(data, 4, "SPK2", 0, offset));
    EXPECT_FALSE(FindBytes(data, 0, "SPK2", 4, offset));
}

TEST(FindBytesTest, NullBuffer)
{
    unsigned int offset = 0;
    EXPECT_FALSE(FindBytes(nullptr, 4, "SPK2", 4, offset));
    EXPECT_FALSE(FindBytes("SPK2", 4, nullptr, 4, offset));
}

TEST(FindBytesTest, MagicSplitAcrossRange)
{
    // 魔术字跨收包边界时，落在范围外的那半个不能算命中
    const char data[] = { 'S', 'P', 'K', '2' };
    unsigned int offset = 0;
    EXPECT_FALSE(FindBytes(data, 2, "SPK2", 4, offset));
}
