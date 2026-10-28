#include <Spark/Serialization/Csv/CsvRecord.h>
#include "doctest/doctest.h"

#include <string_view>
using namespace Spark::Serialization;
// ============================================================
// CsvRecord 测试
// ============================================================

TEST_CASE("CsvRecordTest.BasicAnalysis")
{
    CsvRecord record;
    REQUIRE(record.AnalysisFieldName("name,age,money"));
    REQUIRE(record.AnalysisFieldContent("peter,20,123.5"));

    CHECK_EQ(std::string_view(record.GetFieldAsString("name")), std::string_view("peter"));
    CHECK_EQ(record.GetFieldAsInt("age"), 20);
    CHECK_EQ(record.GetFieldAsDouble("money"), 123.5);
}

TEST_CASE("CsvRecordTest.GetFieldCount")
{
    CsvRecord record;
    record.AnalysisFieldName("a,b,c");
    CHECK_EQ(record.GetFieldCount(), 3);
}

TEST_CASE("CsvRecordTest.EmptyField")
{
    CsvRecord record;
    record.AnalysisFieldName("name,age");
    record.AnalysisFieldContent("peter,");

    CHECK_EQ(std::string_view(record.GetFieldAsString("name")), std::string_view("peter"));
    CHECK_EQ(std::string_view(record.GetFieldAsString("age")), std::string_view(""));
}

TEST_CASE("CsvRecordTest.MissingFieldReturnsNull")
{
    CsvRecord record;
    record.AnalysisFieldName("name,age");
    record.AnalysisFieldContent("peter,20");

    CHECK_EQ(record.GetFieldAsString("nonexistent"), nullptr);
}

TEST_CASE("CsvRecordTest.GetFieldAsInt_Missing")
{
    CsvRecord record;
    record.AnalysisFieldName("name");
    record.AnalysisFieldContent("peter");

    CHECK_EQ(record.GetFieldAsInt("nonexistent"), 0);
}

TEST_CASE("CsvRecordTest.GetFieldAsDouble_Missing")
{
    CsvRecord record;
    record.AnalysisFieldName("name");
    record.AnalysisFieldContent("peter");

    // 文档: 缺失返回 max
    CHECK_EQ(record.GetFieldAsDouble("nonexistent"), std::numeric_limits<double>::max());
}

TEST_CASE("CsvRecordTest.GetFieldAsDouble_Empty")
{
    CsvRecord record;
    record.AnalysisFieldName("value");
    record.AnalysisFieldContent("");

    // 空字符串也返回 max
    CHECK_EQ(record.GetFieldAsDouble("value"), std::numeric_limits<double>::max());
}

TEST_CASE("CsvRecordTest.GetFieldAsChar")
{
    CsvRecord record;
    record.AnalysisFieldName("initial");
    record.AnalysisFieldContent("A");

    CHECK_EQ(record.GetFieldAsChar("initial"), 'A');
}

TEST_CASE("CsvRecordTest.GetFieldAsChar_Missing")
{
    CsvRecord record;
    record.AnalysisFieldName("x");
    record.AnalysisFieldContent("y");

    CHECK_EQ(record.GetFieldAsChar("nonexistent"), '\0');
}

TEST_CASE("CsvRecordTest.CustomSeparator")
{
    CsvRecord record;
    record.SetSeparator('|');
    record.AnalysisFieldName("name|age");
    record.AnalysisFieldContent("peter|25");

    CHECK_EQ(std::string_view(record.GetFieldAsString("name")), std::string_view("peter"));
    CHECK_EQ(record.GetFieldAsInt("age"), 25);
}
