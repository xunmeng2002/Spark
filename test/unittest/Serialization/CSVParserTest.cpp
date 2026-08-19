#include <gtest/gtest.h>
#include <Spark/Serialization/Csv/CsvParser.h>

#include <cstring>
using namespace spark::serialization;
// ============================================================
// CSVParser 测试
// ============================================================

TEST(CSVParserTest, SimpleFields)
{
    CSVParser parser("a,b,c");
    EXPECT_STREQ(parser.GetNextToken(), "a");
    EXPECT_EQ(parser.GetErrorCode(), CPE_HAS_NEXT);
    EXPECT_STREQ(parser.GetNextToken(), "b");
    EXPECT_EQ(parser.GetErrorCode(), CPE_HAS_NEXT);
    EXPECT_STREQ(parser.GetNextToken(), "c");
    EXPECT_EQ(parser.GetErrorCode(), CPE_END);
}

TEST(CSVParserTest, EmptyInput)
{
    CSVParser parser("");
    auto token = parser.GetNextToken();
    EXPECT_STREQ(token, "");
    EXPECT_EQ(parser.GetErrorCode(), CPE_END);
}

TEST(CSVParserTest, SingleField)
{
    CSVParser parser("hello");
    EXPECT_STREQ(parser.GetNextToken(), "hello");
    EXPECT_EQ(parser.GetErrorCode(), CPE_END);
}

TEST(CSVParserTest, QuotedField)
{
    CSVParser parser("\"hello, world\",next");
    EXPECT_STREQ(parser.GetNextToken(), "hello, world");
    EXPECT_EQ(parser.GetErrorCode(), CPE_HAS_NEXT);
    EXPECT_STREQ(parser.GetNextToken(), "next");
    EXPECT_EQ(parser.GetErrorCode(), CPE_END);
}

TEST(CSVParserTest, EscapedQuote)
{
    CSVParser parser("\"\"\"escaped\"\"\",end");
    EXPECT_STREQ(parser.GetNextToken(), "\"escaped\"");
    EXPECT_EQ(parser.GetErrorCode(), CPE_HAS_NEXT);
    EXPECT_STREQ(parser.GetNextToken(), "end");
    EXPECT_EQ(parser.GetErrorCode(), CPE_END);
}

TEST(CSVParserTest, EmptyField)
{
    CSVParser parser("a,,c");
    EXPECT_STREQ(parser.GetNextToken(), "a");
    EXPECT_STREQ(parser.GetNextToken(), "");
    EXPECT_STREQ(parser.GetNextToken(), "c");
    EXPECT_EQ(parser.GetErrorCode(), CPE_END);
}

TEST(CSVParserTest, CustomSeparator)
{
    CSVParser parser("a|b|c");
    parser.SetSeparator('|');
    EXPECT_STREQ(parser.GetNextToken(), "a");
    EXPECT_STREQ(parser.GetNextToken(), "b");
    EXPECT_STREQ(parser.GetNextToken(), "c");
    EXPECT_EQ(parser.GetErrorCode(), CPE_END);
}

TEST(CSVParserTest, ParseReuse)
{
    CSVParser parser("first,second");
    EXPECT_STREQ(parser.GetNextToken(), "first");
    EXPECT_STREQ(parser.GetNextToken(), "second");
    EXPECT_EQ(parser.GetErrorCode(), CPE_END);

    parser.Parse("new1,new2");
    EXPECT_STREQ(parser.GetNextToken(), "new1");
    EXPECT_STREQ(parser.GetNextToken(), "new2");
    EXPECT_EQ(parser.GetErrorCode(), CPE_END);
}

TEST(CSVParserTest, TrailingComma)
{
    CSVParser parser("a,b,");
    EXPECT_STREQ(parser.GetNextToken(), "a");
    EXPECT_STREQ(parser.GetNextToken(), "b");
    EXPECT_STREQ(parser.GetNextToken(), "");
    EXPECT_EQ(parser.GetErrorCode(), CPE_END);
}
