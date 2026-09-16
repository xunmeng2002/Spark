#include <gtest/gtest.h>
#include <Spark/Serialization/Csv/CsvRecord.h>
using namespace Spark::Serialization;
// ============================================================
// CsvRecord 测试
// ============================================================

TEST(CsvRecordTest, BasicAnalysis)
{
    CsvRecord record;
    ASSERT_TRUE(record.AnalysisFieldName("name,age,money"));
    ASSERT_TRUE(record.AnalysisFieldContent("peter,20,123.5"));

    EXPECT_STREQ(record.GetFieldAsString("name"), "peter");
    EXPECT_EQ(record.GetFieldAsInt("age"), 20);
    EXPECT_DOUBLE_EQ(record.GetFieldAsDouble("money"), 123.5);
}

TEST(CsvRecordTest, GetFieldCount)
{
    CsvRecord record;
    record.AnalysisFieldName("a,b,c");
    EXPECT_EQ(record.GetFieldCount(), 3);
}

TEST(CsvRecordTest, EmptyField)
{
    CsvRecord record;
    record.AnalysisFieldName("name,age");
    record.AnalysisFieldContent("peter,");

    EXPECT_STREQ(record.GetFieldAsString("name"), "peter");
    EXPECT_STREQ(record.GetFieldAsString("age"), "");
}

TEST(CsvRecordTest, MissingFieldReturnsNull)
{
    CsvRecord record;
    record.AnalysisFieldName("name,age");
    record.AnalysisFieldContent("peter,20");

    EXPECT_EQ(record.GetFieldAsString("nonexistent"), nullptr);
}

TEST(CsvRecordTest, GetFieldAsInt_Missing)
{
    CsvRecord record;
    record.AnalysisFieldName("name");
    record.AnalysisFieldContent("peter");

    EXPECT_EQ(record.GetFieldAsInt("nonexistent"), 0);
}

TEST(CsvRecordTest, GetFieldAsDouble_Missing)
{
    CsvRecord record;
    record.AnalysisFieldName("name");
    record.AnalysisFieldContent("peter");

    // 文档: 缺失返回 max
    EXPECT_DOUBLE_EQ(record.GetFieldAsDouble("nonexistent"), std::numeric_limits<double>::max());
}

TEST(CsvRecordTest, GetFieldAsDouble_Empty)
{
    CsvRecord record;
    record.AnalysisFieldName("value");
    record.AnalysisFieldContent("");

    // 空字符串也返回 max
    EXPECT_DOUBLE_EQ(record.GetFieldAsDouble("value"), std::numeric_limits<double>::max());
}

TEST(CsvRecordTest, GetFieldAsChar)
{
    CsvRecord record;
    record.AnalysisFieldName("initial");
    record.AnalysisFieldContent("A");

    EXPECT_EQ(record.GetFieldAsChar("initial"), 'A');
}

TEST(CsvRecordTest, GetFieldAsChar_Missing)
{
    CsvRecord record;
    record.AnalysisFieldName("x");
    record.AnalysisFieldContent("y");

    EXPECT_EQ(record.GetFieldAsChar("nonexistent"), '\0');
}

TEST(CsvRecordTest, CustomSeparator)
{
    CsvRecord record;
    record.SetSeparator('|');
    record.AnalysisFieldName("name|age");
    record.AnalysisFieldContent("peter|25");

    EXPECT_STREQ(record.GetFieldAsString("name"), "peter");
    EXPECT_EQ(record.GetFieldAsInt("age"), 25);
}
