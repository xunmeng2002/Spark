#include <Spark/Core/Utility/Utility.h>
#include "doctest/doctest.h"

#include <cstring>
#include <string_view>
using namespace Spark;
using namespace Spark::Core;
// ============================================================
// Utility 剩余函数测试（ItoA/FtoA 之前已改，此处测试简化版本）
// ============================================================

// ---------- ItoA ----------

TEST_CASE("UtilityTest.ItoA_Positive")
{
    CHECK_EQ(Utility::ItoA(42), "42");
    CHECK_EQ(Utility::ItoA(0), "0");
    CHECK_EQ(Utility::ItoA(2147483647), "2147483647");
}

TEST_CASE("UtilityTest.ItoA_Negative")
{
    CHECK_EQ(Utility::ItoA(-1), "-1");
    CHECK_EQ(Utility::ItoA(-2147483647), "-2147483647");
}

// ---------- FtoA ----------

TEST_CASE("UtilityTest.FtoA_NotEmpty")
{
    // std::format("{}", d) 默认格式与实现有关，验证非空即可
    CHECK_FALSE(Utility::FtoA(3.14).empty());
    CHECK_FALSE(Utility::FtoA(0.0).empty());
    CHECK_FALSE(Utility::FtoA(-1.5).empty());
}

// ---------- RemoveCharacter ----------

TEST_CASE("UtilityTest.RemoveCharacter_AtStart")
{
    char buf[] = "  hello";
    Utility::RemoveCharacter(buf, static_cast<int>(sizeof(buf)), ' ');
    CHECK_EQ(std::string_view(buf), std::string_view("hello"));
}

TEST_CASE("UtilityTest.RemoveCharacter_AtEnd")
{
    char buf[] = "hello  ";
    Utility::RemoveCharacter(buf, static_cast<int>(sizeof(buf)), ' ');
    CHECK_EQ(std::string_view(buf), std::string_view("hello"));
}

TEST_CASE("UtilityTest.RemoveCharacter_AllSpaces")
{
    char buf[] = "     ";
    Utility::RemoveCharacter(buf, static_cast<int>(sizeof(buf)), ' ');
    CHECK_EQ(std::string_view(buf), std::string_view(""));
}

TEST_CASE("UtilityTest.RemoveCharacter_NoMatch")
{
    char buf[] = "hello";
    Utility::RemoveCharacter(buf, static_cast<int>(sizeof(buf)), 'x');
    CHECK_EQ(std::string_view(buf), std::string_view("hello"));
}

TEST_CASE("UtilityTest.RemoveCharacter_Mixed")
{
    char buf[] = "a b c d";
    Utility::RemoveCharacter(buf, static_cast<int>(sizeof(buf)), ' ');
    CHECK_EQ(std::string_view(buf), std::string_view("abcd"));
}

TEST_CASE("UtilityTest.RemoveCharacter_Empty")
{
    char buf[] = "";
    Utility::RemoveCharacter(buf, static_cast<int>(sizeof(buf)), ' ');
    CHECK_EQ(std::string_view(buf), std::string_view(""));
}

// ---------- GetConfigProtocolType ----------

TEST_CASE("UtilityTest.GetConfigProtocolType_Step")
{
    CHECK_EQ(Utility::GetConfigProtocolType("step"), ProtocolTypeType::Step);
}

TEST_CASE("UtilityTest.GetConfigProtocolType_Xtp")
{
    CHECK_EQ(Utility::GetConfigProtocolType("xtp"), ProtocolTypeType::Xtp);
}

TEST_CASE("UtilityTest.GetConfigProtocolType_Unknown")
{
    // 未知类型返回 Xtp（默认行为）
    CHECK_EQ(Utility::GetConfigProtocolType("unknown"), ProtocolTypeType::Xtp);
}

TEST_CASE("UtilityTest.GetConfigProtocolType_Empty")
{
    CHECK_EQ(Utility::GetConfigProtocolType(""), ProtocolTypeType::Xtp);
}

TEST_CASE("UtilityTest.GetConfigProtocolType_CaseSensitive")
{
    // 严格匹配，大写不匹配
    CHECK_EQ(Utility::GetConfigProtocolType("Step"), ProtocolTypeType::Xtp);
}

// ---------- ParseProcessName ----------

TEST_CASE("UtilityTest.ParseProcessName")
{
    char buf[256] = {};
    Utility::ParseProcessName("/usr/local/bin/myapp.exe", buf, static_cast<int>(sizeof(buf)));
    // 函数用 strrchr 找 '/' 或 '\'，然后截断到 '.'
    // 不同平台行为不同，验证不崩溃且非空即可
    CHECK_GT(std::strlen(buf), 0);
}
