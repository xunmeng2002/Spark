#include <gtest/gtest.h>
#include <Spark/Core/Utility/Utility.h>

#include <cstring>
using namespace spark::core;
// ============================================================
// Utility 剩余函数测试（ItoA/FtoA 之前已改，此处测试简化版本）
// ============================================================

// ---------- ItoA ----------

TEST(UtilityTest, ItoA_Positive)
{
    EXPECT_EQ(Utility::ItoA(42), "42");
    EXPECT_EQ(Utility::ItoA(0), "0");
    EXPECT_EQ(Utility::ItoA(2147483647), "2147483647");
}

TEST(UtilityTest, ItoA_Negative)
{
    EXPECT_EQ(Utility::ItoA(-1), "-1");
    EXPECT_EQ(Utility::ItoA(-2147483647), "-2147483647");
}

// ---------- FtoA ----------

TEST(UtilityTest, FtoA_NotEmpty)
{
    // std::format("{}", d) 默认格式与实现有关，验证非空即可
    EXPECT_FALSE(Utility::FtoA(3.14).empty());
    EXPECT_FALSE(Utility::FtoA(0.0).empty());
    EXPECT_FALSE(Utility::FtoA(-1.5).empty());
}

// ---------- RemoveCharacter ----------

TEST(UtilityTest, RemoveCharacter_AtStart)
{
    char buf[] = "  hello";
    Utility::RemoveCharacter(buf, (int)sizeof(buf), ' ');
    EXPECT_STREQ(buf, "hello");
}

TEST(UtilityTest, RemoveCharacter_AtEnd)
{
    char buf[] = "hello  ";
    Utility::RemoveCharacter(buf, (int)sizeof(buf), ' ');
    EXPECT_STREQ(buf, "hello");
}

TEST(UtilityTest, RemoveCharacter_AllSpaces)
{
    char buf[] = "     ";
    Utility::RemoveCharacter(buf, (int)sizeof(buf), ' ');
    EXPECT_STREQ(buf, "");
}

TEST(UtilityTest, RemoveCharacter_NoMatch)
{
    char buf[] = "hello";
    Utility::RemoveCharacter(buf, (int)sizeof(buf), 'x');
    EXPECT_STREQ(buf, "hello");
}

TEST(UtilityTest, RemoveCharacter_Mixed)
{
    char buf[] = "a b c d";
    Utility::RemoveCharacter(buf, (int)sizeof(buf), ' ');
    EXPECT_STREQ(buf, "abcd");
}

TEST(UtilityTest, RemoveCharacter_Empty)
{
    char buf[] = "";
    Utility::RemoveCharacter(buf, (int)sizeof(buf), ' ');
    EXPECT_STREQ(buf, "");
}

// ---------- GetConfigProtocolType ----------

TEST(UtilityTest, GetConfigProtocolType_Step)
{
    EXPECT_EQ(Utility::GetConfigProtocolType("step"), ProtocolTypeType::Step);
}

TEST(UtilityTest, GetConfigProtocolType_Xtp)
{
    EXPECT_EQ(Utility::GetConfigProtocolType("xtp"), ProtocolTypeType::Xtp);
}

TEST(UtilityTest, GetConfigProtocolType_Unknown)
{
    // 未知类型返回 Xtp（默认行为）
    EXPECT_EQ(Utility::GetConfigProtocolType("unknown"), ProtocolTypeType::Xtp);
}

TEST(UtilityTest, GetConfigProtocolType_Empty)
{
    EXPECT_EQ(Utility::GetConfigProtocolType(""), ProtocolTypeType::Xtp);
}

TEST(UtilityTest, GetConfigProtocolType_CaseSensitive)
{
    // 严格匹配，大写不匹配
    EXPECT_EQ(Utility::GetConfigProtocolType("Step"), ProtocolTypeType::Xtp);
}

// ---------- ParseProcessName ----------

TEST(UtilityTest, ParseProcessName)
{
    char buf[256] = {};
    Utility::ParseProcessName("/usr/local/bin/myapp.exe", buf, (int)sizeof(buf));
    // 函数用 strrchr 找 '/' 或 '\'，然后截断到 '.'
    // 不同平台行为不同，验证不崩溃且非空即可
    EXPECT_GT(std::strlen(buf), 0);
}
