#include <gtest/gtest.h>
#include <Spark/Serialization/Csv/CsvRecord.h>
using namespace spark::serialization;
// ============================================================
// CSVRecord 测试
// ============================================================

TEST(CSVRecordTest, BasicAnalysis)
{
    CSVRecord record;
    ASSERT_TRUE(record.AnalysisFieldName("name,age,money"));
    ASSERT_TRUE(record.AnalysisFieldContent("peter,20,123.5"));

    EXPECT_STREQ(record.GetFieldAsString("name"), "peter");
    EXPECT_EQ(record.GetFieldAsInt("age"), 20);
    EXPECT_DOUBLE_EQ(record.GetFieldAsDouble("money"), 123.5);
}

TEST(CSVRecordTest, GetFieldCount)
{
    CSVRecord record;
    record.AnalysisFieldName("a,b,c");
    EXPECT_EQ(record.GetFieldCount(), 3);
}

TEST(CSVRecordTest, EmptyField)
{
    CSVRecord record;
    record.AnalysisFieldName("name,age");
    record.AnalysisFieldContent("peter,");

    EXPECT_STREQ(record.GetFieldAsString("name"), "peter");
    EXPECT_STREQ(record.GetFieldAsString("age"), "");
}

TEST(CSVRecordTest, MissingFieldReturnsNull)
{
    CSVRecord record;
    record.AnalysisFieldName("name,age");
    record.AnalysisFieldContent("peter,20");

    EXPECT_EQ(record.GetFieldAsString("nonexistent"), nullptr);
}

TEST(CSVRecordTest, GetFieldAsInt_Missing)
{
    CSVRecord record;
    record.AnalysisFieldName("name");
    record.AnalysisFieldContent("peter");

    EXPECT_EQ(record.GetFieldAsInt("nonexistent"), 0);
}

TEST(CSVRecordTest, GetFieldAsDouble_Missing)
{
    CSVRecord record;
    record.AnalysisFieldName("name");
    record.AnalysisFieldContent("peter");

    // 文档: 缺失返回 max
    EXPECT_DOUBLE_EQ(record.GetFieldAsDouble("nonexistent"), std::numeric_limits<double>::max());
}

TEST(CSVRecordTest, GetFieldAsDouble_Empty)
{
    CSVRecord record;
    record.AnalysisFieldName("value");
    record.AnalysisFieldContent("");

    // 空字符串也返回 max
    EXPECT_DOUBLE_EQ(record.GetFieldAsDouble("value"), std::numeric_limits<double>::max());
}

TEST(CSVRecordTest, GetFieldAsChar)
{
    CSVRecord record;
    record.AnalysisFieldName("initial");
    record.AnalysisFieldContent("A");

    EXPECT_EQ(record.GetFieldAsChar("initial"), 'A');
}

TEST(CSVRecordTest, GetFieldAsChar_Missing)
{
    CSVRecord record;
    record.AnalysisFieldName("x");
    record.AnalysisFieldContent("y");

    EXPECT_EQ(record.GetFieldAsChar("nonexistent"), '\0');
}

TEST(CSVRecordTest, CustomSeparator)
{
    CSVRecord record;
    record.SetSeparator('|');
    record.AnalysisFieldName("name|age");
    record.AnalysisFieldContent("peter|25");

    EXPECT_STREQ(record.GetFieldAsString("name"), "peter");
    EXPECT_EQ(record.GetFieldAsInt("age"), 25);
}
