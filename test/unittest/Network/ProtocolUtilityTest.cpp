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
