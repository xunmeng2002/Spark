#include <gtest/gtest.h>
#include <Spark/Core/MD5/MD5.h>

#include <string>
using namespace spark::core;
// ============================================================
// MD5 测试 — 使用 RFC 1321 标准测试向量
// ============================================================

TEST(MD5Test, EmptyString)
{
    std::string result = getMD5(reinterpret_cast<const unsigned char*>(""), 0);
    EXPECT_EQ(result, "d41d8cd98f00b204e9800998ecf8427e");
}

TEST(MD5Test, StringAbc)
{
    std::string input = "abc";
    std::string result = getMD5(reinterpret_cast<const unsigned char*>(input.c_str()), (int)input.length());
    EXPECT_EQ(result, "900150983cd24fb0d6963f7d28e17f72");
}

TEST(MD5Test, StringMessageDigest)
{
    std::string input = "message digest";
    std::string result = getMD5(reinterpret_cast<const unsigned char*>(input.c_str()), (int)input.length());
    EXPECT_EQ(result, "f96b697d7cb7938d525a2f31aaf161d0");
}

TEST(MD5Test, StringA_Z)
{
    std::string input = "abcdefghijklmnopqrstuvwxyz";
    std::string result = getMD5(reinterpret_cast<const unsigned char*>(input.c_str()), (int)input.length());
    EXPECT_EQ(result, "c3fcd3d76192e4007dfb496cca67e13b");
}

TEST(MD5Test, NumericChars)
{
    std::string input = "12345678901234567890123456789012345678901234567890123456789012345678901234567890";
    std::string result = getMD5(reinterpret_cast<const unsigned char*>(input.c_str()), (int)input.length());
    EXPECT_EQ(result, "57edf4a22be3c955ac49da2e2107b67a");
}

TEST(MD5Test, OutputLength)
{
    std::string input = "HelloWorld";
    std::string result = getMD5(reinterpret_cast<const unsigned char*>(input.c_str()), (int)input.length());
    // MD5 hex string 始终是 32 字符
    EXPECT_EQ(result.length(), 32);
}

TEST(MD5Test, OnlyHexChars)
{
    std::string input = "HelloWorld";
    std::string result = getMD5(reinterpret_cast<const unsigned char*>(input.c_str()), (int)input.length());
    // 只包含 0-9 和 a-f
    for (char c : result)
    {
        EXPECT_TRUE((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'));
    }
}

TEST(MD5Test, Deterministic)
{
    std::string input = "HelloWorld";
    auto r1 = getMD5(reinterpret_cast<const unsigned char*>(input.c_str()), (int)input.length());
    auto r2 = getMD5(reinterpret_cast<const unsigned char*>(input.c_str()), (int)input.length());
    EXPECT_EQ(r1, r2);
}
