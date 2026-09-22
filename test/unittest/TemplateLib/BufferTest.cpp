#include <gtest/gtest.h>
#include <Spark/TemplateLib/Buffer/Buffer.h>

#include <cstring>
#include <string>
#include <type_traits>
using namespace Spark;
// ============================================================
// Buffer 测试 — 固定大小缓冲区（Append / SetLength / Shift / Reset / MemMove）
// ============================================================

static constexpr size_t kBufferSize = 64;

// ---------- 构造 / 初始状态 ----------

TEST(BufferTest, DefaultConstructor)
{
    Buffer<kBufferSize> buf;
    EXPECT_EQ(buf.GetLength(), 0u);
    EXPECT_EQ(buf.GetData(), buf.GetWritePos());
}

// ---------- Append ----------

TEST(BufferTest, Append_Basic)
{
    Buffer<kBufferSize> buf;
    const char* data = "Hello";
    size_t written = buf.Append(data, 5);
    EXPECT_EQ(written, 5u);
    EXPECT_EQ(buf.GetLength(), 5u);
    EXPECT_EQ(std::memcmp(buf.GetData(), data, 5), 0);
}

TEST(BufferTest, Append_ExactFit)
{
    Buffer<kBufferSize> buf;
    std::string data(kBufferSize, 'A');
    size_t written = buf.Append(data.data(), kBufferSize);
    EXPECT_EQ(written, kBufferSize);
    EXPECT_EQ(buf.GetLength(), kBufferSize);
}

TEST(BufferTest, Append_Overflow)
{
    Buffer<kBufferSize> buf;
    std::string data(kBufferSize + 10, 'B');
    size_t written = buf.Append(data.data(), data.size());
    // Should truncate to available space
    EXPECT_EQ(written, kBufferSize);
    EXPECT_EQ(buf.GetLength(), kBufferSize);
}

TEST(BufferTest, Append_Multiple)
{
    Buffer<kBufferSize> buf;
    EXPECT_EQ(buf.Append("AAA", 3), 3u);
    EXPECT_EQ(buf.Append("BBB", 3), 3u);
    EXPECT_EQ(buf.GetLength(), 6u);
    EXPECT_EQ(std::memcmp(buf.GetData(), "AAABBB", 6), 0);
}

TEST(BufferTest, Append_ZeroLengthKeepsState)
{
    Buffer<kBufferSize> buf;
    buf.Append("Hello", 5);
    EXPECT_EQ(buf.Append("", 0), 0u);
    EXPECT_EQ(buf.GetLength(), 5u);
    EXPECT_EQ(buf.GetWriteBufferSize(), kBufferSize - 5);
    EXPECT_EQ(std::memcmp(buf.GetData(), "Hello", 5), 0);
}

// ---------- Shift ----------

TEST(BufferTest, Shift_Partial)
{
    Buffer<kBufferSize> buf;
    buf.Append("HelloWorld", 10);
    buf.Shift(5);
    EXPECT_EQ(buf.GetLength(), 5u);
    EXPECT_EQ(std::memcmp(buf.GetData(), "World", 5), 0);
}

TEST(BufferTest, Shift_All)
{
    Buffer<kBufferSize> buf;
    buf.Append("Hello", 5);
    buf.Shift(5);
    EXPECT_EQ(buf.GetLength(), 0u);
    // ReadPos should be reset to beginning
    EXPECT_EQ(buf.GetData(), buf.GetWritePos());
}

TEST(BufferTest, Shift_Excess)
{
    Buffer<kBufferSize> buf;
    buf.Append("Hello", 5);
    buf.Shift(10); // more than length
    EXPECT_EQ(buf.GetLength(), 0u);
}

TEST(BufferTest, Shift_Zero)
{
    Buffer<kBufferSize> buf;
    buf.Append("Hello", 5);
    buf.Shift(0);
    EXPECT_EQ(buf.GetLength(), 5u);
}

// ---------- Shift + Append (环形复用) ----------

TEST(BufferTest, ShiftThenMemMoveThenAppend_ReusesSpace)
{
    Buffer<kBufferSize> buf;
    buf.Append(std::string(kBufferSize, 'X').data(), kBufferSize);
    EXPECT_EQ(buf.GetLength(), kBufferSize);

    // Shift half — 但 Buffer 是线性缓冲区，Shift 只移动读指针，写位置不变
    buf.Shift(kBufferSize / 2);
    EXPECT_EQ(buf.GetLength(), kBufferSize / 2);

    // 此时写指针在末尾，写空间为 0
    EXPECT_EQ(buf.GetWriteBufferSize(), 0u);

    // MemMove 将剩余数据紧贴到头部，释放写空间
    buf.MemMove();
    EXPECT_GT(buf.GetWriteBufferSize(), 0u);

    size_t written = buf.Append("YYY", 3);
    EXPECT_EQ(written, 3u);
    EXPECT_EQ(buf.GetLength(), kBufferSize / 2 + 3);
}

// ---------- SetLength ----------

// 已写入 10 字节、已消费 5 字节：读位置到缓冲末尾只剩 kBufferSize - 5 字节
class PartiallyConsumedBufferTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        buffer_.Append("HelloWorld", 10);
        buffer_.Shift(5);
    }

    void ExpectFilledToEndOfCapacity()
    {
        EXPECT_EQ(buffer_.GetLength(), kBufferSize - 5);
        EXPECT_EQ(buffer_.GetWriteBufferSize(), 0u);
        EXPECT_EQ(buffer_.Append("Z", 1), 0u);
    }

    Buffer<kBufferSize> buffer_;
};

TEST(BufferTest, SetLength)
{
    Buffer<kBufferSize> buf;
    buf.SetLength(10);
    EXPECT_EQ(buf.GetLength(), 10u);
    buf.SetLength(0);
    EXPECT_EQ(buf.GetLength(), 0u);
}

TEST_F(PartiallyConsumedBufferTest, SetLength_AtCapacityFillsWriteWindow)
{
    // 从读位置到缓冲末尾的容量 = 剩余长度 + 写窗口；登记满容量后写窗口应为 0
    const size_t capacity = buffer_.GetLength() + buffer_.GetWriteBufferSize();
    ASSERT_EQ(capacity, kBufferSize - 5);
    buffer_.SetLength(capacity);
    ExpectFilledToEndOfCapacity();
}

TEST_F(PartiallyConsumedBufferTest, SetLength_ReportsLengthFromReadPosition)
{
    // 登记的是「从读位置起的有效字节数」，不搬移数据、也不是写入增量
    buffer_.SetLength(3);
    EXPECT_EQ(buffer_.GetLength(), 3u);
    EXPECT_EQ(buffer_.GetWriteBufferSize(), kBufferSize - 8);
    EXPECT_EQ(std::memcmp(buffer_.GetData(), "Wor", 3), 0);
    EXPECT_EQ(buffer_.Append("ZZ", 2), 2u);
    EXPECT_EQ(buffer_.GetLength(), 5u);
}

#if defined(GTEST_HAS_DEATH_TEST) && !defined(NDEBUG)
TEST_F(PartiallyConsumedBufferTest, SetLength_BeyondCapacityTripsAssert)
{
    // 超出读位置到缓冲末尾的距离，Debug 构建下必须由断言当场拦下
    ASSERT_DEATH(buffer_.SetLength(kBufferSize), "len <= capacity");
}
#endif

#ifdef NDEBUG
// 断言关闭后钳位是唯一防线，故本用例只在 Release 构建下有意义
TEST_F(PartiallyConsumedBufferTest, SetLength_BeyondCapacityClampsWithoutAssert)
{
    buffer_.SetLength(kBufferSize);
    ExpectFilledToEndOfCapacity();
}
#endif

// ---------- GetWriteBufferSize ----------

TEST(BufferTest, GetWriteBufferSize)
{
    Buffer<kBufferSize> buf;
    // Initially: full buffer available
    EXPECT_EQ(buf.GetWriteBufferSize(), kBufferSize);

    buf.Append("Hello", 5);
    EXPECT_EQ(buf.GetWriteBufferSize(), kBufferSize - 5);

    buf.Shift(3);
    // Length = 2, write position = ReadPos + Length = buf+3+2 = buf+5
    // WriteBufferSize = (buf + 64) - (buf + 5) = 59
    EXPECT_EQ(buf.GetWriteBufferSize(), kBufferSize - 5);
}

// ---------- MemMove ----------

TEST(BufferTest, MemMove_CompactsToFront)
{
    Buffer<kBufferSize> buf;
    // Fill, shift some, then MemMove should compact remaining to front
    buf.Append(std::string(kBufferSize, 'A').data(), kBufferSize);
    buf.Shift(kBufferSize - 10); // keep last 10 bytes
    EXPECT_EQ(buf.GetLength(), 10u);

    buf.MemMove();
    EXPECT_EQ(buf.GetLength(), 10u);
    // After MemMove, data should be at the beginning of the buffer
    EXPECT_EQ(buf.GetData(), buf.GetWritePos() - 10);
}

TEST(BufferTest, MemMove_EmptyBuffer)
{
    Buffer<kBufferSize> buf;
    buf.MemMove(); // should be a no-op
    EXPECT_EQ(buf.GetLength(), 0u);
}

TEST(BufferTest, MemMove_EnablesMoreWrites)
{
    Buffer<kBufferSize> buf;

    // Fill, shift most, then MemMove to get write space back
    buf.Append(std::string(kBufferSize, 'A').data(), kBufferSize);
    buf.Shift(kBufferSize / 2);
    buf.Append(std::string(kBufferSize / 2, 'B').data(), kBufferSize / 2);
    // Now the buffer is "full" in terms of write window:
    // ReadPos is at buf + 32, Length = 32, so write position is at buf + 64 = end
    EXPECT_EQ(buf.GetWriteBufferSize(), 0u);

    // But there's free space at the front (0..31) — MemMove fixes that
    buf.MemMove();
    EXPECT_GT(buf.GetWriteBufferSize(), 0u);
}

// ---------- 拷贝 / 移动 ----------

TEST(BufferTest, CopyAndMoveAreDeleted)
{
    EXPECT_FALSE(std::is_copy_constructible_v<Buffer<kBufferSize>>);
    EXPECT_FALSE(std::is_copy_assignable_v<Buffer<kBufferSize>>);
    EXPECT_FALSE(std::is_move_constructible_v<Buffer<kBufferSize>>);
    EXPECT_FALSE(std::is_move_assignable_v<Buffer<kBufferSize>>);
}

// ---------- Reset ----------

TEST(BufferTest, Reset)
{
    Buffer<kBufferSize> buf;
    buf.Append("Hello", 5);
    buf.Shift(2);
    buf.Reset();
    EXPECT_EQ(buf.GetLength(), 0u);
    EXPECT_EQ(buf.GetData(), buf.GetWritePos());
}

// ---------- 完整工作流 ----------

TEST(BufferTest, FullWorkflow)
{
    Buffer<kBufferSize> buf;

    // Append data
    EXPECT_EQ(buf.Append("Hello", 5), 5u);

    // Shift part
    buf.Shift(2);
    EXPECT_EQ(buf.GetLength(), 3u);

    // Append more
    EXPECT_EQ(buf.Append("World", 5), 5u);
    EXPECT_EQ(buf.GetLength(), 8u);

    // MemMove to compact
    buf.MemMove();

    // Clear and reuse
    buf.Reset();
    EXPECT_EQ(buf.GetLength(), 0u);
    EXPECT_EQ(buf.Append("Final", 5), 5u);
    EXPECT_EQ(buf.GetLength(), 5u);
}
