#include <Spark/Serialization/Base64/Base64.h>
#include "doctest/doctest.h"

#include <cstring>
#include <vector>
using namespace Spark::Serialization;
// ============================================================
// Base64 测试 — RFC 4648 标准测试向量
// ============================================================

// 辅助：编码到 string
static std::string EncodeToString(const std::string& input)
{
    std::vector<unsigned char> output((input.length() + 2) / 3 * 4 + 1);
    int destLen = 0;
    Base64Encode(reinterpret_cast<const unsigned char*>(input.data()), static_cast<int>(input.length()), output.data(), destLen);
    return std::string(reinterpret_cast<const char*>(output.data()), destLen);
}

// 辅助：解码到 string
static std::string DecodeToString(const std::string& input)
{
    std::vector<unsigned char> output(input.length());
    int destLen = 0;
    Base64Decode(reinterpret_cast<const unsigned char*>(input.data()), static_cast<int>(input.length()), output.data(), destLen);
    return std::string(reinterpret_cast<const char*>(output.data()), destLen);
}

// ---------- RFC 4648 编码测试 ----------

TEST_CASE("Base64Test.Encode_Empty")
{
    CHECK_EQ(EncodeToString(""), "");
}

TEST_CASE("Base64Test.Encode_f")
{
    CHECK_EQ(EncodeToString("f"), "Zg==");
}

TEST_CASE("Base64Test.Encode_fo")
{
    CHECK_EQ(EncodeToString("fo"), "Zm8=");
}

TEST_CASE("Base64Test.Encode_foo")
{
    CHECK_EQ(EncodeToString("foo"), "Zm9v");
}

TEST_CASE("Base64Test.Encode_foob")
{
    CHECK_EQ(EncodeToString("foob"), "Zm9vYg==");
}

TEST_CASE("Base64Test.Encode_fooba")
{
    CHECK_EQ(EncodeToString("fooba"), "Zm9vYmE=");
}

TEST_CASE("Base64Test.Encode_foobar")
{
    CHECK_EQ(EncodeToString("foobar"), "Zm9vYmFy");
}

// ---------- RFC 4648 解码测试 ----------

TEST_CASE("Base64Test.Decode_Empty")
{
    CHECK_EQ(DecodeToString(""), "");
}

TEST_CASE("Base64Test.Decode_Zg")
{
    CHECK_EQ(DecodeToString("Zg=="), "f");
}

TEST_CASE("Base64Test.Decode_Zm8")
{
    CHECK_EQ(DecodeToString("Zm8="), "fo");
}

TEST_CASE("Base64Test.Decode_Zm9v")
{
    CHECK_EQ(DecodeToString("Zm9v"), "foo");
}

// ---------- 编解码互逆 ----------

TEST_CASE("Base64Test.RoundTrip_Empty")
{
    std::string original = "";
    auto encoded = EncodeToString(original);
    auto decoded = DecodeToString(encoded);
    CHECK_EQ(original, decoded);
}

TEST_CASE("Base64Test.RoundTrip_Short")
{
    std::string original = "Hello";
    auto encoded = EncodeToString(original);
    auto decoded = DecodeToString(encoded);
    CHECK_EQ(original, decoded);
}

TEST_CASE("Base64Test.RoundTrip_AllBytes")
{
    std::string original;
    for (int i = 0; i < 256; ++i)
        original.push_back(static_cast<char>(i));

    auto encoded = EncodeToString(original);
    auto decoded = DecodeToString(encoded);
    CHECK_EQ(original, decoded);
}

TEST_CASE("Base64Test.RoundTrip_Binary")
{
    unsigned char raw[] = {0x00, 0xFF, 0x80, 0x7F, 0x55, 0xAA};
    std::string original(reinterpret_cast<const char*>(raw), sizeof(raw));
    auto encoded = EncodeToString(original);
    auto decoded = DecodeToString(encoded);
    CHECK_EQ(original, decoded);
}
