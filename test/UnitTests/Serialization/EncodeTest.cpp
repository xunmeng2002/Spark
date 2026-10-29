#include <Spark/Serialization/Encode/Encode.h>
#include "doctest/doctest.h"
using namespace Spark::Serialization;
// ============================================================
// Encode 测试 — GBK/UTF-8/Unicode 互转（仅 ASCII 范围可跨平台断言）
// ============================================================

TEST_CASE("EncodeTest.Utf8ToUnicode_Ascii")
{
    std::wstring result = Utf8ToUnicode("Hello");
    CHECK_EQ(result.length(), 5);
    CHECK_EQ(result[0], L'H');
    CHECK_EQ(result[4], L'o');
}

TEST_CASE("EncodeTest.UnicodeToUtf8_Ascii")
{
    std::wstring wstr = L"Hello";
    std::string result = UnicodeToUtf8(wstr);
    CHECK_EQ(result, "Hello");
}

TEST_CASE("EncodeTest.Utf8RoundTrip_Ascii")
{
    std::string original = "Hello, World!";
    std::wstring uni = Utf8ToUnicode(original);
    std::string back = UnicodeToUtf8(uni);
    CHECK_EQ(original, back);
}

TEST_CASE("EncodeTest.GbkToUnicode_Ascii")
{
    // GBK 对 ASCII 兼容，所有单字节 ASCII 字符编解码一致
    std::wstring result = GbkToUnicode("Hello");
    CHECK_EQ(result.length(), 5);
    CHECK_EQ(result[0], L'H');
}

TEST_CASE("EncodeTest.UnicodeToGbk_Ascii")
{
    std::wstring wstr = L"Hello";
    std::string result = UnicodeToGbk(wstr);
    CHECK_EQ(result, "Hello");
}

TEST_CASE("EncodeTest.GbkRoundTrip_Ascii")
{
    std::string original = "Hello, World!";
    std::wstring uni = GbkToUnicode(original);
    std::string back = UnicodeToGbk(uni);
    CHECK_EQ(original, back);
}

TEST_CASE("EncodeTest.EmptyString")
{
    CHECK(Utf8ToUnicode("").empty());
    CHECK(UnicodeToUtf8(L"").empty());
    CHECK(GbkToUnicode("").empty());
    CHECK(UnicodeToGbk(L"").empty());
}

TEST_CASE("EncodeTest.GbkToUtf8_Ascii")
{
    // ASCII 范围 GBK 和 UTF-8 相同
    CHECK_EQ(GbkToUtf8("Hello"), "Hello");
}

TEST_CASE("EncodeTest.Utf8ToGbk_Ascii")
{
    CHECK_EQ(Utf8ToGbk("Hello"), "Hello");
}
