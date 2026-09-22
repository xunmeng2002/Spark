#include <gtest/gtest.h>
#include <Spark/TemplateLib/Buffer/SpscRingBuffer.h>

#include <cstring>
#include <string>
#include <vector>
using namespace Spark;
// ============================================================
// SpscRingBuffer 测试 — 单生产者单消费者环形缓冲区（Write / Read / Peek / Skip）
// ============================================================

static constexpr size_t SpscRingBufferSize = 64;

// 按流位置生成字节：位置与内容一一对应，用于检出乱序、丢字节、重复字节
static char ByteAt(size_t streamPosition)
{
    return static_cast<char>((streamPosition * 31 + 7) % 251);
}

// ---------- 构造 / 初始状态 ----------

TEST(SpscRingBufferTest, DefaultConstructor_Empty)
{
    SpscRingBuffer<SpscRingBufferSize> buf;
    EXPECT_TRUE(buf.IsEmpty());
    EXPECT_FALSE(buf.IsFull());
    EXPECT_EQ(buf.GetReadBufferSize(), 0u);
    EXPECT_EQ(buf.GetWriteBufferSize(), SpscRingBufferSize);
}

TEST(SpscRingBufferTest, SmallestCapacity)
{
    SpscRingBuffer<1> buf;
    EXPECT_EQ(buf.Write("A", 1), 1u);
    EXPECT_TRUE(buf.IsFull());
    EXPECT_FALSE(buf.IsEmpty());

    char output[1] = {};
    EXPECT_EQ(buf.Read(output, 1), 1u);
    EXPECT_EQ(output[0], 'A');
    EXPECT_TRUE(buf.IsEmpty());
}

TEST(SpscRingBufferTest, SmallestMultiSlotCapacity)
{
    SpscRingBuffer<2> buf;
    ASSERT_EQ(buf.Write("AB", 2), 2u);
    EXPECT_TRUE(buf.IsFull());

    char first[2] = {};
    ASSERT_EQ(buf.Read(first, 1), 1u);
    EXPECT_EQ(first[0], 'A');

    // 写位置绕回 0；读出时应按环序取到 1 号槽的 B、再绕回 0 号槽的 C
    ASSERT_EQ(buf.Write("C", 1), 1u);
    char rest[2] = {};
    EXPECT_EQ(buf.Read(rest, 2), 2u);
    EXPECT_EQ(rest[0], 'B');
    EXPECT_EQ(rest[1], 'C');
}

// ---------- Write / Read ----------

TEST(SpscRingBufferTest, WriteAndRead)
{
    SpscRingBuffer<SpscRingBufferSize> buf;
    const char* data = "Hello";
    EXPECT_EQ(buf.Write(data, 5), 5u);
    EXPECT_FALSE(buf.IsEmpty());
    EXPECT_EQ(buf.GetReadBufferSize(), 5u);

    char output[16] = {};
    EXPECT_EQ(buf.Read(output, 5), 5u);
    EXPECT_EQ(std::memcmp(output, "Hello", 5), 0);
    EXPECT_TRUE(buf.IsEmpty());
}

TEST(SpscRingBufferTest, Write_FillToFull)
{
    SpscRingBuffer<SpscRingBufferSize> buf;
    std::string data(SpscRingBufferSize, 'X');
    EXPECT_EQ(buf.Write(data.data(), data.size()), SpscRingBufferSize);
    EXPECT_TRUE(buf.IsFull());
    EXPECT_EQ(buf.GetWriteBufferSize(), 0u);
}

TEST(SpscRingBufferTest, Write_TruncatesToRemainingSpace)
{
    SpscRingBuffer<SpscRingBufferSize> buf;
    std::string firstPart(SpscRingBufferSize - 4, 'A');
    EXPECT_EQ(buf.Write(firstPart.data(), firstPart.size()), SpscRingBufferSize - 4);

    // 缓冲区仅剩 4 字节空间，写入 10 字节只落 4 字节
    EXPECT_EQ(buf.Write("BBBBBBBBBB", 10), 4u);
    EXPECT_TRUE(buf.IsFull());
    EXPECT_EQ(buf.GetReadBufferSize(), SpscRingBufferSize);
}

TEST(SpscRingBufferTest, Write_WhenFullWritesNothing)
{
    SpscRingBuffer<SpscRingBufferSize> buf;
    std::string data(SpscRingBufferSize, 'X');
    ASSERT_EQ(buf.Write(data.data(), data.size()), SpscRingBufferSize);
    EXPECT_EQ(buf.Write("Y", 1), 0u);
    EXPECT_TRUE(buf.IsFull());
}

TEST(SpscRingBufferTest, Write_ZeroLength)
{
    SpscRingBuffer<SpscRingBufferSize> buf;
    EXPECT_EQ(buf.Write("Hello", 0), 0u);
    EXPECT_TRUE(buf.IsEmpty());
}

TEST(SpscRingBufferTest, Read_Empty)
{
    SpscRingBuffer<SpscRingBufferSize> buf;
    char output[16] = {};
    EXPECT_EQ(buf.Read(output, 5), 0u);
}

TEST(SpscRingBufferTest, Read_ZeroLength)
{
    SpscRingBuffer<SpscRingBufferSize> buf;
    buf.Write("Hello", 5);
    char output[16] = {};
    EXPECT_EQ(buf.Read(output, 0), 0u);
    EXPECT_EQ(buf.GetReadBufferSize(), 5u);
}

TEST(SpscRingBufferTest, Read_TruncatesToReadableBytes)
{
    SpscRingBuffer<SpscRingBufferSize> buf;
    buf.Write("Hello", 5);

    char output[16] = {};
    EXPECT_EQ(buf.Read(output, 16), 5u);
    EXPECT_EQ(std::memcmp(output, "Hello", 5), 0);
    EXPECT_TRUE(buf.IsEmpty());
}

TEST(SpscRingBufferTest, Read_MultipleChunks)
{
    SpscRingBuffer<SpscRingBufferSize> buf;
    std::string payload(SpscRingBufferSize, '\0');
    for (size_t i = 0; i < payload.size(); ++i)
    {
        payload[i] = ByteAt(i);
    }
    ASSERT_EQ(buf.Write(payload.data(), payload.size()), SpscRingBufferSize);

    std::vector<char> received;
    char chunk[7] = {};
    size_t readLength = 0;
    while ((readLength = buf.Read(chunk, sizeof(chunk))) > 0)
    {
        received.insert(received.end(), chunk, chunk + readLength);
    }

    EXPECT_EQ(received.size(), payload.size());
    EXPECT_EQ(std::memcmp(received.data(), payload.data(), payload.size()), 0);
}

// ---------- Peek ----------

TEST(SpscRingBufferTest, Peek_DoesNotConsume)
{
    SpscRingBuffer<SpscRingBufferSize> buf;
    buf.Write("Hello", 5);

    char first[16] = {};
    EXPECT_EQ(buf.Peek(first, 5), 5u);
    EXPECT_EQ(std::memcmp(first, "Hello", 5), 0);

    // 再 Peek 一次，数据仍在
    char second[16] = {};
    EXPECT_EQ(buf.Peek(second, 5), 5u);
    EXPECT_EQ(std::memcmp(second, "Hello", 5), 0);

    EXPECT_EQ(buf.GetReadBufferSize(), 5u);
}

TEST(SpscRingBufferTest, Peek_TruncatesToReadableBytes)
{
    SpscRingBuffer<SpscRingBufferSize> buf;
    buf.Write("Hello", 5);

    char output[16] = {};
    EXPECT_EQ(buf.Peek(output, 16), 5u);
    EXPECT_EQ(std::memcmp(output, "Hello", 5), 0);
    EXPECT_EQ(buf.GetReadBufferSize(), 5u);
}

TEST(SpscRingBufferTest, Peek_Empty)
{
    SpscRingBuffer<SpscRingBufferSize> buf;
    char output[16] = {};
    EXPECT_EQ(buf.Peek(output, 5), 0u);
}

TEST(SpscRingBufferTest, Peek_ZeroLength)
{
    SpscRingBuffer<SpscRingBufferSize> buf;
    buf.Write("Hello", 5);

    char output[16] = {};
    EXPECT_EQ(buf.Peek(output, 0), 0u);
    EXPECT_EQ(buf.GetReadBufferSize(), 5u);
}

TEST(SpscRingBufferTest, Peek_WrapsAround)
{
    SpscRingBuffer<SpscRingBufferSize> buf;

    // 先把读索引推到 10：写入 10 字节后全部读走
    std::string consumed(10, 'Z');
    ASSERT_EQ(buf.Write(consumed.data(), consumed.size()), consumed.size());
    char discard[16] = {};
    ASSERT_EQ(buf.Read(discard, consumed.size()), consumed.size());

    // 再写 60 字节：越过末尾 54 字节后绕回缓冲区头部，Peek 需按环序拼接
    std::string payload(SpscRingBufferSize - 4, '\0');
    for (size_t i = 0; i < payload.size(); ++i)
    {
        payload[i] = ByteAt(i);
    }
    ASSERT_EQ(buf.Write(payload.data(), payload.size()), payload.size());

    std::vector<char> peeked(SpscRingBufferSize, '\0');
    EXPECT_EQ(buf.Peek(peeked.data(), peeked.size()), payload.size());
    EXPECT_EQ(std::memcmp(peeked.data(), payload.data(), payload.size()), 0);
    EXPECT_EQ(buf.GetReadBufferSize(), payload.size());
}

TEST(SpscRingBufferTest, PeekThenRead_Equal)
{
    SpscRingBuffer<SpscRingBufferSize> buf;
    buf.Write("HelloWorld", 10);

    char peeked[16] = {};
    ASSERT_EQ(buf.Peek(peeked, 10), 10u);

    char read[16] = {};
    ASSERT_EQ(buf.Read(read, 10), 10u);
    EXPECT_EQ(std::memcmp(peeked, read, 10), 0);
}

// ---------- Skip ----------

TEST(SpscRingBufferTest, Skip)
{
    SpscRingBuffer<SpscRingBufferSize> buf;
    buf.Write("HelloWorld", 10);

    EXPECT_EQ(buf.Skip(5), 5u);
    EXPECT_EQ(buf.GetReadBufferSize(), 5u);

    char output[16] = {};
    EXPECT_EQ(buf.Read(output, 5), 5u);
    EXPECT_EQ(std::memcmp(output, "World", 5), 0);
}

TEST(SpscRingBufferTest, Skip_ExcessClampsToReadableBytes)
{
    SpscRingBuffer<SpscRingBufferSize> buf;
    buf.Write("Hello", 5);
    EXPECT_EQ(buf.Skip(100), 5u);
    EXPECT_TRUE(buf.IsEmpty());
}

TEST(SpscRingBufferTest, Skip_ZeroLength)
{
    SpscRingBuffer<SpscRingBufferSize> buf;
    buf.Write("Hello", 5);
    EXPECT_EQ(buf.Skip(0), 0u);
    EXPECT_EQ(buf.GetReadBufferSize(), 5u);
}

TEST(SpscRingBufferTest, Skip_ThenReadAcrossWrapBoundary)
{
    SpscRingBuffer<SpscRingBufferSize> buf;

    std::string payload(SpscRingBufferSize, 'X');
    ASSERT_EQ(buf.Write(payload.data(), payload.size()), SpscRingBufferSize);
    char discard[4] = {};
    ASSERT_EQ(buf.Read(discard, 2), 2u);

    // 读走 2 字节后只剩 2 字节空间，写入的 2 字节落到绕回后的 0..1 号槽
    ASSERT_EQ(buf.Write("YZ", 2), 2u);
    EXPECT_TRUE(buf.IsFull());

    // 跳过 61 字节使读位置落到 63 号槽；再读 3 字节必须跨过末尾拼接 Y、Z
    EXPECT_EQ(buf.Skip(61), 61u);
    char output[4] = {};
    EXPECT_EQ(buf.Read(output, 3), 3u);
    EXPECT_EQ(std::memcmp(output, "XYZ", 3), 0);
    EXPECT_TRUE(buf.IsEmpty());
}

TEST(SpscRingBufferTest, PeekThenSkip_EqualsRead)
{
    SpscRingBuffer<SpscRingBufferSize> buf;
    buf.Write("HelloWorld", 10);

    char peeked[16] = {};
    ASSERT_EQ(buf.Peek(peeked, 6), 6u);
    ASSERT_EQ(buf.Skip(6), 6u);
    EXPECT_EQ(buf.GetReadBufferSize(), 4u);

    char rest[16] = {};
    ASSERT_EQ(buf.Read(rest, 16), 4u);
    EXPECT_EQ(std::memcmp(peeked, "HelloW", 6), 0);
    EXPECT_EQ(std::memcmp(rest, "orld", 4), 0);
}

// ---------- 绕回（wrap-around） ----------

TEST(SpscRingBufferTest, Write_WrapsAroundBoundary)
{
    SpscRingBuffer<SpscRingBufferSize> buf;

    // 写 61 字节后读走 2 字节：写位置落在 61，剩余可写 5 字节，再写 5 字节必然跨过末尾
    std::string headPart(61, 'A');
    ASSERT_EQ(buf.Write(headPart.data(), headPart.size()), 61u);
    char discard[4] = {};
    ASSERT_EQ(buf.Read(discard, 2), 2u);
    ASSERT_EQ(buf.GetWriteBufferSize(), 5u);

    // 3 字节落在 61..63 号槽，2 字节绕回 0..1 号槽
    ASSERT_EQ(buf.Write("BBBBB", 5), 5u);
    EXPECT_TRUE(buf.IsFull());
    EXPECT_EQ(buf.GetReadBufferSize(), SpscRingBufferSize);
    EXPECT_EQ(buf.GetWriteBufferSize(), 0u);

    // 读出全部：首段 62 字节（59 个 A + 3 个 B），余段 2 字节（B）
    std::vector<char> output(SpscRingBufferSize);
    EXPECT_EQ(buf.Read(output.data(), SpscRingBufferSize), SpscRingBufferSize);

    std::string expected(59, 'A');
    expected += "BBBBB";
    EXPECT_EQ(std::memcmp(output.data(), expected.data(), SpscRingBufferSize), 0);
}

TEST(SpscRingBufferTest, Read_WrapsAround)
{
    SpscRingBuffer<SpscRingBufferSize> buf;

    std::string payload(SpscRingBufferSize, 'X');
    ASSERT_EQ(buf.Write(payload.data(), payload.size()), SpscRingBufferSize);
    EXPECT_TRUE(buf.IsFull());

    // 读走 5 字节释放队头空间
    char discard[16] = {};
    ASSERT_EQ(buf.Read(discard, 5), 5u);
    EXPECT_EQ(buf.GetWriteBufferSize(), 5u);

    // 写满释放出的 5 字节
    ASSERT_EQ(buf.Write("YYYYY", 5), 5u);
    EXPECT_TRUE(buf.IsFull());

    std::vector<char> output(SpscRingBufferSize);
    EXPECT_EQ(buf.Read(output.data(), SpscRingBufferSize), SpscRingBufferSize);

    EXPECT_EQ(output[0], 'X');
    for (size_t i = SpscRingBufferSize - 5; i < SpscRingBufferSize; ++i)
    {
        EXPECT_EQ(output[i], 'Y');
    }
}

TEST(SpscRingBufferTest, AccumulatingPartialReadsPreserveByteOrder)
{
    SpscRingBuffer<SpscRingBufferSize> buf;

    // 每轮写 7 读 5：可读量逐轮累积、读写位置持续绕回；每 10 轮请求整容量，可读量不足故必触发部分读
    constexpr size_t WriteLength = 7;
    constexpr size_t DrainedRoundInterval = 10;
    std::vector<char> received;
    size_t writtenTotal = 0;

    for (size_t round = 0; round < 300; ++round)
    {
        char payload[WriteLength] = {};
        for (size_t i = 0; i < WriteLength; ++i)
        {
            payload[i] = ByteAt(writtenTotal + i);
        }
        ASSERT_EQ(buf.Write(payload, WriteLength), WriteLength);
        writtenTotal += WriteLength;

        const bool isDrainedRound = ((round + 1) % DrainedRoundInterval) == 0;
        const size_t requestedLength = isDrainedRound ? SpscRingBufferSize : 5;
        char chunk[SpscRingBufferSize] = {};
        const size_t readLength = buf.Read(chunk, requestedLength);
        ASSERT_LE(readLength, requestedLength);
        if (isDrainedRound)
        {
            ASSERT_LT(readLength, requestedLength);
        }
        received.insert(received.end(), chunk, chunk + readLength);
    }

    // 读出的字节是写入流的前缀，且没有字节被吞掉
    ASSERT_EQ(received.size() + buf.GetReadBufferSize(), writtenTotal);
    for (size_t i = 0; i < received.size(); ++i)
    {
        ASSERT_EQ(received[i], ByteAt(i)) << "stream position " << i;
    }
}

// ---------- 多个 Write/Read 周期 ----------

TEST(SpscRingBufferTest, MultiCycle)
{
    SpscRingBuffer<SpscRingBufferSize> buf;

    for (int cycle = 0; cycle < 10; ++cycle)
    {
        std::string data = "Cycle" + std::to_string(cycle);
        EXPECT_EQ(buf.Write(data.data(), data.size()), data.size());

        char output[32] = {};
        EXPECT_EQ(buf.Read(output, data.size()), data.size());
        EXPECT_EQ(std::memcmp(output, data.data(), data.size()), 0);
        EXPECT_TRUE(buf.IsEmpty());
    }
}

// ---------- ResetWhenIdle ----------

TEST(SpscRingBufferTest, ResetWhenIdle_AfterWrapAround)
{
    SpscRingBuffer<SpscRingBufferSize> buf;

    // 先让读写索引双双超过容量（绕回态）
    std::string payload(SpscRingBufferSize, 'X');
    ASSERT_EQ(buf.Write(payload.data(), payload.size()), SpscRingBufferSize);
    char discard[SpscRingBufferSize] = {};
    ASSERT_EQ(buf.Read(discard, 5), 5u);
    ASSERT_EQ(buf.Write("YYYYY", 5), 5u);
    ASSERT_EQ(buf.Read(discard, SpscRingBufferSize), SpscRingBufferSize);
    ASSERT_TRUE(buf.IsEmpty());

    buf.ResetWhenIdle();
    EXPECT_TRUE(buf.IsEmpty());
    EXPECT_FALSE(buf.IsFull());
    EXPECT_EQ(buf.GetWriteBufferSize(), SpscRingBufferSize);

    // 复位后仍可正常读写
    EXPECT_EQ(buf.Write("World", 5), 5u);
    char output[16] = {};
    EXPECT_EQ(buf.Read(output, 5), 5u);
    EXPECT_EQ(std::memcmp(output, "World", 5), 0);
}
