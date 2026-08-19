#include <gtest/gtest.h>
#include <Spark/Network/Protocol/PackageReader.h>

#include <cstring>
using namespace spark::network;
// ============================================================
// PackageReader 缓冲管理测试
// 仅测试缓冲操作方法（Append/PopFront/Shift/Reset 等），
// 不涉及 ParsePackage（需要 PackageFactory 和协议数据）
// ============================================================

namespace
{
    // 辅助常量
    constexpr const char* kTestIP = "127.0.0.1";

    // 构建一个只测试缓冲管理的 PackageReader
    PackageReader MakeReader()
    {
        return PackageReader(ProtocolTypeType::Step, nullptr, 0, kTestIP);
    }
}

TEST(PackageReaderTest, InitialState)
{
    PackageReader reader = MakeReader();

    EXPECT_EQ(reader.Length(), 0);
    EXPECT_NE(reader.Data(), nullptr);
    // Data 应为内部缓冲起始地址
    EXPECT_EQ(reader.Data(), reader.Tail());
}

TEST(PackageReaderTest, ResetAfterConstruction)
{
    PackageReader reader = MakeReader();
    reader.Reset();

    EXPECT_EQ(reader.Length(), 0);
    EXPECT_EQ(reader.Data(), reader.Tail());
}

TEST(PackageReaderTest, AppendAndReadData)
{
    PackageReader reader = MakeReader();
    const char testData[] = "Hello, PackageReader!";

    unsigned int appended = reader.Append(const_cast<char*>(testData),
                                           (unsigned int)strlen(testData));
    EXPECT_EQ(appended, strlen(testData));
    EXPECT_EQ(reader.Length(), (int)strlen(testData));

    // Data() 应指向刚写入的数据
    EXPECT_EQ(memcmp(reader.Data(), testData, strlen(testData)), 0);
}

TEST(PackageReaderTest, AppendUpdatesTail)
{
    PackageReader reader = MakeReader();

    const char data1[] = "First";
    reader.Append(const_cast<char*>(data1), (unsigned int)strlen(data1));
    EXPECT_EQ(reader.Length(), (int)strlen(data1));

    const char data2[] = "Second";
    reader.Append(const_cast<char*>(data2), (unsigned int)strlen(data2));
    EXPECT_EQ(reader.Length(), (int)(strlen(data1) + strlen(data2)));

    // Tail() 应在所有数据之后
    EXPECT_EQ(reader.Tail(), reader.Data() + reader.Length());
}

TEST(PackageReaderTest, PopFrontRemovesData)
{
    PackageReader reader = MakeReader();
    const char testData[] = "HelloWorld";
    reader.Append(const_cast<char*>(testData), (unsigned int)strlen(testData));

    reader.PopFront(5);  // remove "Hello"
    EXPECT_EQ(reader.Length(), 5);
    EXPECT_EQ(memcmp(reader.Data(), "World", 5), 0);  // "World" remains
}

TEST(PackageReaderTest, PopFrontClampsToLength)
{
    PackageReader reader = MakeReader();
    const char testData[] = "Short";
    reader.Append(const_cast<char*>(testData), (unsigned int)strlen(testData));

    // PopFront 超过实际长度——应 clamp 到 m_Length
    reader.PopFront(100);
    EXPECT_EQ(reader.Length(), 0);
}

TEST(PackageReaderTest, PopFrontAllData)
{
    PackageReader reader = MakeReader();
    const char testData[] = "TestData";
    reader.Append(const_cast<char*>(testData), (unsigned int)strlen(testData));

    reader.PopFront((unsigned int)strlen(testData));
    EXPECT_EQ(reader.Length(), 0);
    EXPECT_EQ(reader.Data(), reader.Tail());
}

TEST(PackageReaderTest, PopFrontMovesDataToBufferStart)
{
    PackageReader reader = MakeReader();

    // 连续 Append 和 PopFront 后数据应始终位于 m_Buff 起始
    const char data1[] = "ABCDE";
    const char data2[] = "FGH";

    reader.Append(const_cast<char*>(data1), (unsigned int)strlen(data1));
    EXPECT_EQ(reader.Data(), reader.Tail() - reader.Length());

    reader.PopFront(2);  // remove "AB" → "CDE" moved to start
    EXPECT_EQ(memcmp(reader.Data(), "CDE", 3), 0);

    reader.Append(const_cast<char*>(data2), (unsigned int)strlen(data2));
    // "CDE" + "FGH" = "CDEFGH"
    EXPECT_EQ(memcmp(reader.Data(), "CDEFGH", 6), 0);
}

TEST(PackageReaderTest, ShiftMovesPointer)
{
    PackageReader reader = MakeReader();
    const char testData[] = "LongerTestData";
    reader.Append(const_cast<char*>(testData), (unsigned int)strlen(testData));

    char* before = reader.Data();
    int beforeLen = reader.Length();

    reader.Shift(6);

    // Shift 仅移动指针，不改缓冲
    EXPECT_EQ(reader.Data(), before + 6);
    EXPECT_EQ(reader.Length(), beforeLen - 6);
}

TEST(PackageReaderTest, ShiftAndAppend)
{
    PackageReader reader = MakeReader();

    const char data[] = "ABCDEFGH";
    reader.Append(const_cast<char*>(data), (unsigned int)strlen(data));
    reader.Shift(4);  // now pointer at "EFGH"

    // 追加在新位置之后
    const char more[] = "IJK";
    reader.Append(const_cast<char*>(more), (unsigned int)strlen(more));
    EXPECT_EQ(memcmp(reader.Data(), "EFGHIJK", 7), 0);
}

TEST(PackageReaderTest, TailSizeDecreasesAsDataGrows)
{
    PackageReader reader = MakeReader();
    unsigned int initialTailSize = reader.TailSize();

    const char data[] = "SomeData";
    int dataLen = (int)strlen(data);
    reader.Append(const_cast<char*>(data), dataLen);

    // 追加数据后 TailSize 应减少
    EXPECT_EQ(reader.TailSize(), initialTailSize - dataLen);
}

TEST(PackageReaderTest, AppendClampsToTailSize)
{
    PackageReader reader = MakeReader();

    // 大量追加，验证不会超过尾端容量
    unsigned int tailSize = reader.TailSize();
    std::string bigData(tailSize + 100, 'X');
    unsigned int appended = reader.Append(const_cast<char*>(bigData.data()),
                                           (unsigned int)bigData.size());
    // 应该被 clamp 到 tailSize
    EXPECT_EQ(appended, tailSize);
    EXPECT_EQ(reader.Length(), (int)tailSize);
}

TEST(PackageReaderTest, ResetRestoresState)
{
    PackageReader reader = MakeReader();

    const char data[] = "SomeData";
    reader.Append(const_cast<char*>(data), (unsigned int)strlen(data));
    reader.Shift(2);
    EXPECT_GT(reader.Length(), 0);

    reader.Reset();
    EXPECT_EQ(reader.Length(), 0);
    EXPECT_EQ(reader.Data(), reader.Tail());
}

TEST(PackageReaderTest, MultiplePopFrontAndVerifyData)
{
    PackageReader reader = MakeReader();

    // 多轮追加 → 弹出 → 验证数据完整性
    const char* expected = "HelloReaderTest";
    reader.Append(const_cast<char*>(expected),
                   (unsigned int)strlen(expected));

    reader.PopFront(5);   // remove "Hello"
    EXPECT_EQ(memcmp(reader.Data(), "ReaderTest", 10), 0);

    reader.PopFront(6);   // remove "Reader"
    EXPECT_EQ(memcmp(reader.Data(), "Test", 4), 0);

    reader.PopFront(4);   // remove "Test"
    EXPECT_EQ(reader.Length(), 0);
}
