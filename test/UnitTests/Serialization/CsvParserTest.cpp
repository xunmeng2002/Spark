#include <Spark/Serialization/Csv/CsvParser.h>
#include "doctest/doctest.h"

#include <cstring>
#include <string_view>
using namespace Spark::Serialization;
// ============================================================
// CsvParser 测试
// ============================================================

TEST_CASE("CsvParserTest.SimpleFields")
{
    CsvParser parser("a,b,c");
    CHECK_EQ(std::string_view(parser.GetNextToken()), std::string_view("a"));
    CHECK_EQ(parser.GetErrorCode(), CsvParserError::HasNext);
    CHECK_EQ(std::string_view(parser.GetNextToken()), std::string_view("b"));
    CHECK_EQ(parser.GetErrorCode(), CsvParserError::HasNext);
    CHECK_EQ(std::string_view(parser.GetNextToken()), std::string_view("c"));
    CHECK_EQ(parser.GetErrorCode(), CsvParserError::End);
}

TEST_CASE("CsvParserTest.EmptyInput")
{
    CsvParser parser("");
    auto token = parser.GetNextToken();
    CHECK_EQ(std::string_view(token), std::string_view(""));
    CHECK_EQ(parser.GetErrorCode(), CsvParserError::End);
}

TEST_CASE("CsvParserTest.SingleField")
{
    CsvParser parser("hello");
    CHECK_EQ(std::string_view(parser.GetNextToken()), std::string_view("hello"));
    CHECK_EQ(parser.GetErrorCode(), CsvParserError::End);
}

TEST_CASE("CsvParserTest.QuotedField")
{
    CsvParser parser("\"hello, world\",next");
    CHECK_EQ(std::string_view(parser.GetNextToken()), std::string_view("hello, world"));
    CHECK_EQ(parser.GetErrorCode(), CsvParserError::HasNext);
    CHECK_EQ(std::string_view(parser.GetNextToken()), std::string_view("next"));
    CHECK_EQ(parser.GetErrorCode(), CsvParserError::End);
}

TEST_CASE("CsvParserTest.EscapedQuote")
{
    CsvParser parser("\"\"\"escaped\"\"\",end");
    CHECK_EQ(std::string_view(parser.GetNextToken()), std::string_view("\"escaped\""));
    CHECK_EQ(parser.GetErrorCode(), CsvParserError::HasNext);
    CHECK_EQ(std::string_view(parser.GetNextToken()), std::string_view("end"));
    CHECK_EQ(parser.GetErrorCode(), CsvParserError::End);
}

TEST_CASE("CsvParserTest.EmptyField")
{
    CsvParser parser("a,,c");
    CHECK_EQ(std::string_view(parser.GetNextToken()), std::string_view("a"));
    CHECK_EQ(std::string_view(parser.GetNextToken()), std::string_view(""));
    CHECK_EQ(std::string_view(parser.GetNextToken()), std::string_view("c"));
    CHECK_EQ(parser.GetErrorCode(), CsvParserError::End);
}

TEST_CASE("CsvParserTest.CustomSeparator")
{
    CsvParser parser("a|b|c");
    parser.SetSeparator('|');
    CHECK_EQ(std::string_view(parser.GetNextToken()), std::string_view("a"));
    CHECK_EQ(std::string_view(parser.GetNextToken()), std::string_view("b"));
    CHECK_EQ(std::string_view(parser.GetNextToken()), std::string_view("c"));
    CHECK_EQ(parser.GetErrorCode(), CsvParserError::End);
}

TEST_CASE("CsvParserTest.ParseReuse")
{
    CsvParser parser("first,second");
    CHECK_EQ(std::string_view(parser.GetNextToken()), std::string_view("first"));
    CHECK_EQ(std::string_view(parser.GetNextToken()), std::string_view("second"));
    CHECK_EQ(parser.GetErrorCode(), CsvParserError::End);

    parser.Parse("new1,new2");
    CHECK_EQ(std::string_view(parser.GetNextToken()), std::string_view("new1"));
    CHECK_EQ(std::string_view(parser.GetNextToken()), std::string_view("new2"));
    CHECK_EQ(parser.GetErrorCode(), CsvParserError::End);
}

TEST_CASE("CsvParserTest.TrailingComma")
{
    CsvParser parser("a,b,");
    CHECK_EQ(std::string_view(parser.GetNextToken()), std::string_view("a"));
    CHECK_EQ(std::string_view(parser.GetNextToken()), std::string_view("b"));
    CHECK_EQ(std::string_view(parser.GetNextToken()), std::string_view(""));
    CHECK_EQ(parser.GetErrorCode(), CsvParserError::End);
}
