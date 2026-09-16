#include <gtest/gtest.h>
#include <Spark/TemplateLib/Buffer/RingBuffer.h>

#include <cstring>
#include <numeric>
#include <string>
#include <vector>
using namespace Spark;
// ============================================================
// RingBuffer 测试 — 环形缓冲区（Write / Read / Peek / Skip）
// ============================================================

static constexpr unsigned RingBufferSize = 64;

// ---------- 构造 / 初始状态 ----------

TEST(RingBufferTest, DefaultConstructor)
{
    RingBuffer<RingBufferSize> buf;
    EXPECT_TRUE(buf.IsEmpty());
    EXPECT_FALSE(buf.IsFull());
    EXPECT_EQ(buf.GetReadBufferSize(), 0u);
    EXPECT_EQ(buf.GetWriteBufferSize(), RingBufferSize);
}

// ---------- Write / Read ----------

TEST(RingBufferTest, WriteAndRead)
{
    RingBuffer<RingBufferSize> buf;
    const char* data = "Hello";
    unsigned written = buf.Write(data, 5);
    EXPECT_EQ(written, 5u);
    EXPECT_FALSE(buf.IsEmpty());
    EXPECT_EQ(buf.GetReadBufferSize(), 5u);

    char output[16] = {};
    unsigned read = buf.Read(output, 5);
    EXPECT_EQ(read, 5u);
    EXPECT_EQ(std::memcmp(output, "Hello", 5), 0);
    EXPECT_TRUE(buf.IsEmpty());
}

TEST(RingBufferTest, Write_FillToFull)
{
    RingBuffer<RingBufferSize> buf;
    std::string data(RingBufferSize, 'X');
    unsigned written = buf.Write(data.data(), RingBufferSize);
    EXPECT_EQ(written, RingBufferSize);
    EXPECT_TRUE(buf.IsFull());
}

TEST(RingBufferTest, Write_Overflow)
{
    RingBuffer<RingBufferSize> buf;
    std::string data(RingBufferSize + 10, 'X');
    unsigned written = buf.Write(data.data(), static_cast<unsigned>(data.size()));
    EXPECT_EQ(written, RingBufferSize); // truncated
    EXPECT_TRUE(buf.IsFull());
}

TEST(RingBufferTest, Read_Empty)
{
    RingBuffer<RingBufferSize> buf;
    char output[16] = {};
    EXPECT_EQ(buf.Read(output, 5), 0u);
}

// ---------- Peek ----------

TEST(RingBufferTest, Peek_DoesNotConsume)
{
    RingBuffer<RingBufferSize> buf;
    buf.Write("Hello", 5);

    char first[16] = {};
    EXPECT_EQ(buf.Peek(first, 5), 5u);
    EXPECT_EQ(std::memcmp(first, "Hello", 5), 0);

    // Peek again — data still there
    char second[16] = {};
    EXPECT_EQ(buf.Peek(second, 5), 5u);
    EXPECT_EQ(std::memcmp(second, "Hello", 5), 0);

    // Read still has data
    EXPECT_EQ(buf.GetReadBufferSize(), 5u);
}

// ---------- Skip ----------

TEST(RingBufferTest, Skip)
{
    RingBuffer<RingBufferSize> buf;
    buf.Write("HelloWorld", 10);

    EXPECT_EQ(buf.Skip(5), 5u);
    EXPECT_EQ(buf.GetReadBufferSize(), 5u);

    char output[16] = {};
    EXPECT_EQ(buf.Read(output, 5), 5u);
    EXPECT_EQ(std::memcmp(output, "World", 5), 0);
}

TEST(RingBufferTest, Skip_Excess)
{
    RingBuffer<RingBufferSize> buf;
    buf.Write("Hello", 5);
    EXPECT_EQ(buf.Skip(100), 5u);
    EXPECT_TRUE(buf.IsEmpty());
}

// ---------- 绕回（wrap-around） ----------

TEST(RingBufferTest, Write_WrapsAround)
{
    RingBuffer<RingBufferSize> buf;

    // Write data that forces wrap-around: fill to near end, then write more
    std::string first(RingBufferSize - 5, 'A');
    buf.Write(first.data(), static_cast<unsigned>(first.size()));
    EXPECT_FALSE(buf.IsFull());

    // Write more — should wrap to the beginning
    buf.Write("BBBBB", 5);
    EXPECT_TRUE(buf.IsFull());

    // Now read all data and verify order
    std::vector<char> output(RingBufferSize);
    EXPECT_EQ(buf.Read(output.data(), RingBufferSize), RingBufferSize);

    std::string expected(RingBufferSize - 5, 'A');
    expected += "BBBBB";
    EXPECT_EQ(std::memcmp(output.data(), expected.data(), RingBufferSize), 0);
}

TEST(RingBufferTest, Read_WrapsAround)
{
    RingBuffer<RingBufferSize> buf;

    // Fill entirely
    std::string payload(RingBufferSize, 'X');
    buf.Write(payload.data(), RingBufferSize);
    EXPECT_TRUE(buf.IsFull());

    // Read 5 from front, freeing 5 bytes
    char discard[16] = {};
    EXPECT_EQ(buf.Read(discard, 5), 5u);
    EXPECT_EQ(buf.GetWriteBufferSize(), 5u);

    // Write 5 to the freed space (wrapping around)
    buf.Write("YYYYY", 5);
    EXPECT_TRUE(buf.IsFull());

    // Read all — first RingBufferSize-5 of original 'X', then 5 'Y' at the end
    std::vector<char> output(RingBufferSize);
    EXPECT_EQ(buf.Read(output.data(), RingBufferSize), RingBufferSize);

    EXPECT_EQ(output[0], 'X'); // still from original fill
    for (int i = RingBufferSize - 5; i < RingBufferSize; ++i)
    {
        EXPECT_EQ(output[i], 'Y');
    }
}

// ---------- 多个 Write/Read 周期 ----------

TEST(RingBufferTest, MultiCycle)
{
    RingBuffer<RingBufferSize> buf;

    for (int cycle = 0; cycle < 10; ++cycle)
    {
        std::string data = "Cycle" + std::to_string(cycle);
        unsigned written = buf.Write(data.data(), static_cast<unsigned>(data.size()));
        EXPECT_EQ(written, data.size());

        char output[32] = {};
        unsigned read = buf.Read(output, static_cast<unsigned>(data.size()));
        EXPECT_EQ(read, data.size());
        EXPECT_EQ(std::memcmp(output, data.data(), data.size()), 0);
        EXPECT_TRUE(buf.IsEmpty());
    }
}

// ---------- Reset ----------

TEST(RingBufferTest, Reset)
{
    RingBuffer<RingBufferSize> buf;
    buf.Write("Hello", 5);
    buf.Reset();
    EXPECT_TRUE(buf.IsEmpty());
    EXPECT_FALSE(buf.IsFull());
    EXPECT_EQ(buf.GetWriteBufferSize(), RingBufferSize);
}
