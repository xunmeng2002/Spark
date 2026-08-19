#include <gtest/gtest.h>
#include <Spark/Serialization/Encode/Encode.h>
using namespace spark::serialization;
// ============================================================
// Encode 测试 — GBK/UTF-8/Unicode 互转（仅 ASCII 范围可跨平台断言）
// ============================================================

TEST(EncodeTest, Utf8ToUnicode_Ascii)
{
    std::wstring result = Utf8ToUnicode("Hello");
    EXPECT_EQ(result.length(), 5);
    EXPECT_EQ(result[0], L'H');
    EXPECT_EQ(result[4], L'o');
}

TEST(EncodeTest, UnicodeToUtf8_Ascii)
{
    std::wstring wstr = L"Hello";
    std::string result = UnicodeToUtf8(wstr);
    EXPECT_EQ(result, "Hello");
}

TEST(EncodeTest, Utf8RoundTrip_Ascii)
{
    std::string original = "Hello, World!";
    std::wstring uni = Utf8ToUnicode(original);
    std::string back = UnicodeToUtf8(uni);
    EXPECT_EQ(original, back);
}

TEST(EncodeTest, GbkToUnicode_Ascii)
{
    // GBK 对 ASCII 兼容，所有单字节 ASCII 字符编解码一致
    std::wstring result = GbkToUnicode("Hello");
    EXPECT_EQ(result.length(), 5);
    EXPECT_EQ(result[0], L'H');
}

TEST(EncodeTest, UnicodeToGbk_Ascii)
{
    std::wstring wstr = L"Hello";
    std::string result = UnicodeToGbk(wstr);
    EXPECT_EQ(result, "Hello");
}

TEST(EncodeTest, GbkRoundTrip_Ascii)
{
    std::string original = "Hello, World!";
    std::wstring uni = GbkToUnicode(original);
    std::string back = UnicodeToGbk(uni);
    EXPECT_EQ(original, back);
}

TEST(EncodeTest, EmptyString)
{
    EXPECT_TRUE(Utf8ToUnicode("").empty());
    EXPECT_TRUE(UnicodeToUtf8(L"").empty());
    EXPECT_TRUE(GbkToUnicode("").empty());
    EXPECT_TRUE(UnicodeToGbk(L"").empty());
}

TEST(EncodeTest, GbkToUtf8_Ascii)
{
    // ASCII 范围 GBK 和 UTF-8 相同
    EXPECT_EQ(GbkToUtf8("Hello"), "Hello");
}

TEST(EncodeTest, Utf8ToGbk_Ascii)
{
    EXPECT_EQ(Utf8ToGbk("Hello"), "Hello");
}
