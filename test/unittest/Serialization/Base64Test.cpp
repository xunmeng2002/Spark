#include <gtest/gtest.h>
#include <Spark/Serialization/Base64/Base64.h>

#include <cstring>
#include <vector>
using namespace spark::serialization;
// ============================================================
// Base64 测试 — RFC 4648 标准测试向量
// ============================================================

// 辅助：编码到 string
static std::string EncodeToString(const std::string& input)
{
    std::vector<unsigned char> output((input.length() + 2) / 3 * 4 + 1);
    int destLen = 0;
    Base64Encode(
        reinterpret_cast<const unsigned char*>(input.data()),
        (int)input.length(),
        output.data(),
        destLen);
    return std::string(reinterpret_cast<const char*>(output.data()), destLen);
}

// 辅助：解码到 string
static std::string DecodeToString(const std::string& input)
{
    std::vector<unsigned char> output(input.length());
    int destLen = 0;
    Base64Decode(
        reinterpret_cast<const unsigned char*>(input.data()),
        (int)input.length(),
        output.data(),
        destLen);
    return std::string(reinterpret_cast<const char*>(output.data()), destLen);
}

// ---------- RFC 4648 编码测试 ----------

TEST(Base64Test, Encode_Empty)
{
    EXPECT_EQ(EncodeToString(""), "");
}

TEST(Base64Test, Encode_f)
{
    EXPECT_EQ(EncodeToString("f"), "Zg==");
}

TEST(Base64Test, Encode_fo)
{
    EXPECT_EQ(EncodeToString("fo"), "Zm8=");
}

TEST(Base64Test, Encode_foo)
{
    EXPECT_EQ(EncodeToString("foo"), "Zm9v");
}

TEST(Base64Test, Encode_foob)
{
    EXPECT_EQ(EncodeToString("foob"), "Zm9vYg==");
}

TEST(Base64Test, Encode_fooba)
{
    EXPECT_EQ(EncodeToString("fooba"), "Zm9vYmE=");
}

TEST(Base64Test, Encode_foobar)
{
    EXPECT_EQ(EncodeToString("foobar"), "Zm9vYmFy");
}

// ---------- RFC 4648 解码测试 ----------

TEST(Base64Test, Decode_Empty)
{
    EXPECT_EQ(DecodeToString(""), "");
}

TEST(Base64Test, Decode_Zg)
{
    EXPECT_EQ(DecodeToString("Zg=="), "f");
}

TEST(Base64Test, Decode_Zm8)
{
    EXPECT_EQ(DecodeToString("Zm8="), "fo");
}

TEST(Base64Test, Decode_Zm9v)
{
    EXPECT_EQ(DecodeToString("Zm9v"), "foo");
}

// ---------- 编解码互逆 ----------

TEST(Base64Test, RoundTrip_Empty)
{
    std::string original = "";
    auto encoded = EncodeToString(original);
    auto decoded = DecodeToString(encoded);
    EXPECT_EQ(original, decoded);
}

TEST(Base64Test, RoundTrip_Short)
{
    std::string original = "Hello";
    auto encoded = EncodeToString(original);
    auto decoded = DecodeToString(encoded);
    EXPECT_EQ(original, decoded);
}

TEST(Base64Test, RoundTrip_AllBytes)
{
    std::string original;
    for (int i = 0; i < 256; ++i)
        original.push_back(static_cast<char>(i));

    auto encoded = EncodeToString(original);
    auto decoded = DecodeToString(encoded);
    EXPECT_EQ(original, decoded);
}

TEST(Base64Test, RoundTrip_Binary)
{
    unsigned char raw[] = { 0x00, 0xFF, 0x80, 0x7F, 0x55, 0xAA };
    std::string original(reinterpret_cast<const char*>(raw), sizeof(raw));
    auto encoded = EncodeToString(original);
    auto decoded = DecodeToString(encoded);
    EXPECT_EQ(original, decoded);
}
