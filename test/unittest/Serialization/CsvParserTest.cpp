#include <gtest/gtest.h>
#include <Spark/Serialization/Csv/CsvParser.h>

#include <cstring>
using namespace Spark::Serialization;
// ============================================================
// CsvParser 测试
// ============================================================

TEST(CsvParserTest, SimpleFields)
{
    CsvParser parser("a,b,c");
    EXPECT_STREQ(parser.GetNextToken(), "a");
    EXPECT_EQ(parser.GetErrorCode(), CsvParserError::HasNext);
    EXPECT_STREQ(parser.GetNextToken(), "b");
    EXPECT_EQ(parser.GetErrorCode(), CsvParserError::HasNext);
    EXPECT_STREQ(parser.GetNextToken(), "c");
    EXPECT_EQ(parser.GetErrorCode(), CsvParserError::End);
}

TEST(CsvParserTest, EmptyInput)
{
    CsvParser parser("");
    auto token = parser.GetNextToken();
    EXPECT_STREQ(token, "");
    EXPECT_EQ(parser.GetErrorCode(), CsvParserError::End);
}

TEST(CsvParserTest, SingleField)
{
    CsvParser parser("hello");
    EXPECT_STREQ(parser.GetNextToken(), "hello");
    EXPECT_EQ(parser.GetErrorCode(), CsvParserError::End);
}

TEST(CsvParserTest, QuotedField)
{
    CsvParser parser("\"hello, world\",next");
    EXPECT_STREQ(parser.GetNextToken(), "hello, world");
    EXPECT_EQ(parser.GetErrorCode(), CsvParserError::HasNext);
    EXPECT_STREQ(parser.GetNextToken(), "next");
    EXPECT_EQ(parser.GetErrorCode(), CsvParserError::End);
}

TEST(CsvParserTest, EscapedQuote)
{
    CsvParser parser("\"\"\"escaped\"\"\",end");
    EXPECT_STREQ(parser.GetNextToken(), "\"escaped\"");
    EXPECT_EQ(parser.GetErrorCode(), CsvParserError::HasNext);
    EXPECT_STREQ(parser.GetNextToken(), "end");
    EXPECT_EQ(parser.GetErrorCode(), CsvParserError::End);
}

TEST(CsvParserTest, EmptyField)
{
    CsvParser parser("a,,c");
    EXPECT_STREQ(parser.GetNextToken(), "a");
    EXPECT_STREQ(parser.GetNextToken(), "");
    EXPECT_STREQ(parser.GetNextToken(), "c");
    EXPECT_EQ(parser.GetErrorCode(), CsvParserError::End);
}

TEST(CsvParserTest, CustomSeparator)
{
    CsvParser parser("a|b|c");
    parser.SetSeparator('|');
    EXPECT_STREQ(parser.GetNextToken(), "a");
    EXPECT_STREQ(parser.GetNextToken(), "b");
    EXPECT_STREQ(parser.GetNextToken(), "c");
    EXPECT_EQ(parser.GetErrorCode(), CsvParserError::End);
}

TEST(CsvParserTest, ParseReuse)
{
    CsvParser parser("first,second");
    EXPECT_STREQ(parser.GetNextToken(), "first");
    EXPECT_STREQ(parser.GetNextToken(), "second");
    EXPECT_EQ(parser.GetErrorCode(), CsvParserError::End);

    parser.Parse("new1,new2");
    EXPECT_STREQ(parser.GetNextToken(), "new1");
    EXPECT_STREQ(parser.GetNextToken(), "new2");
    EXPECT_EQ(parser.GetErrorCode(), CsvParserError::End);
}

TEST(CsvParserTest, TrailingComma)
{
    CsvParser parser("a,b,");
    EXPECT_STREQ(parser.GetNextToken(), "a");
    EXPECT_STREQ(parser.GetNextToken(), "b");
    EXPECT_STREQ(parser.GetNextToken(), "");
    EXPECT_EQ(parser.GetErrorCode(), CsvParserError::End);
}
