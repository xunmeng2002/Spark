#include <gtest/gtest.h>
#include <Spark/TemplateLib/Buffer/LinearBuffer.h>

#include <cstring>
#include <string>
#include <type_traits>
using namespace Spark;
// ============================================================
// LinearBuffer 测试 — 固定大小缓冲区（Append / SetLength / Shift / Reset / MemMove）
// ============================================================

static constexpr size_t TestBufferSize = 64;

// ---------- 构造 / 初始状态 ----------

TEST(LinearBufferTest, DefaultConstructor)
{
    LinearBuffer<TestBufferSize> buf;
    EXPECT_EQ(buf.GetLength(), 0u);
    EXPECT_EQ(buf.GetData(), buf.GetWritePos());
}

// ---------- Append ----------

TEST(LinearBufferTest, Append_Basic)
{
    LinearBuffer<TestBufferSize> buf;
    const char* data = "Hello";
    size_t written = buf.Append(data, 5);
    EXPECT_EQ(written, 5u);
    EXPECT_EQ(buf.GetLength(), 5u);
    EXPECT_EQ(std::memcmp(buf.GetData(), data, 5), 0);
}

TEST(LinearBufferTest, Append_ExactFit)
{
    LinearBuffer<TestBufferSize> buf;
    std::string data(TestBufferSize, 'A');
    size_t written = buf.Append(data.data(), TestBufferSize);
    EXPECT_EQ(written, TestBufferSize);
    EXPECT_EQ(buf.GetLength(), TestBufferSize);
}

TEST(LinearBufferTest, Append_Overflow)
{
    LinearBuffer<TestBufferSize> buf;
    std::string data(TestBufferSize + 10, 'B');
    size_t written = buf.Append(data.data(), data.size());
    // Should truncate to available space
    EXPECT_EQ(written, TestBufferSize);
    EXPECT_EQ(buf.GetLength(), TestBufferSize);
}

TEST(LinearBufferTest, Append_Multiple)
{
    LinearBuffer<TestBufferSize> buf;
    EXPECT_EQ(buf.Append("AAA", 3), 3u);
    EXPECT_EQ(buf.Append("BBB", 3), 3u);
    EXPECT_EQ(buf.GetLength(), 6u);
    EXPECT_EQ(std::memcmp(buf.GetData(), "AAABBB", 6), 0);
}

TEST(LinearBufferTest, Append_ZeroLengthKeepsState)
{
    LinearBuffer<TestBufferSize> buf;
    buf.Append("Hello", 5);
    EXPECT_EQ(buf.Append("", 0), 0u);
    EXPECT_EQ(buf.GetLength(), 5u);
    EXPECT_EQ(buf.GetWriteBufferSize(), TestBufferSize - 5);
    EXPECT_EQ(std::memcmp(buf.GetData(), "Hello", 5), 0);
}

// ---------- Shift ----------

TEST(LinearBufferTest, Shift_Partial)
{
    LinearBuffer<TestBufferSize> buf;
    buf.Append("HelloWorld", 10);
    buf.Shift(5);
    EXPECT_EQ(buf.GetLength(), 5u);
    EXPECT_EQ(std::memcmp(buf.GetData(), "World", 5), 0);
}

TEST(LinearBufferTest, Shift_All)
{
    LinearBuffer<TestBufferSize> buf;
    buf.Append("Hello", 5);
    buf.Shift(5);
    EXPECT_EQ(buf.GetLength(), 0u);
    // ReadPos should be reset to beginning
    EXPECT_EQ(buf.GetData(), buf.GetWritePos());
}

TEST(LinearBufferTest, Shift_Excess)
{
    LinearBuffer<TestBufferSize> buf;
    buf.Append("Hello", 5);
    buf.Shift(10); // more than length
    EXPECT_EQ(buf.GetLength(), 0u);
}

TEST(LinearBufferTest, Shift_Zero)
{
    LinearBuffer<TestBufferSize> buf;
    buf.Append("Hello", 5);
    buf.Shift(0);
    EXPECT_EQ(buf.GetLength(), 5u);
}

// ---------- Shift + Append (环形复用) ----------

TEST(LinearBufferTest, ShiftThenMemMoveThenAppend_ReusesSpace)
{
    LinearBuffer<TestBufferSize> buf;
    buf.Append(std::string(TestBufferSize, 'X').data(), TestBufferSize);
    EXPECT_EQ(buf.GetLength(), TestBufferSize);

    // Shift half — 但 LinearBuffer 是线性缓冲区，Shift 只移动读指针，写位置不变
    buf.Shift(TestBufferSize / 2);
    EXPECT_EQ(buf.GetLength(), TestBufferSize / 2);

    // 此时写指针在末尾，写空间为 0
    EXPECT_EQ(buf.GetWriteBufferSize(), 0u);

    // MemMove 将剩余数据紧贴到头部，释放写空间
    buf.MemMove();
    EXPECT_GT(buf.GetWriteBufferSize(), 0u);

    size_t written = buf.Append("YYY", 3);
    EXPECT_EQ(written, 3u);
    EXPECT_EQ(buf.GetLength(), TestBufferSize / 2 + 3);
}

// ---------- SetLength ----------

// 已写入 10 字节、已消费 5 字节：读位置到缓冲末尾只剩 TestBufferSize - 5 字节
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
        EXPECT_EQ(buffer_.GetLength(), TestBufferSize - 5);
        EXPECT_EQ(buffer_.GetWriteBufferSize(), 0u);
        EXPECT_EQ(buffer_.Append("Z", 1), 0u);
    }

    LinearBuffer<TestBufferSize> buffer_;
};

TEST(LinearBufferTest, SetLength)
{
    LinearBuffer<TestBufferSize> buf;
    EXPECT_TRUE(buf.SetLength(10));
    EXPECT_EQ(buf.GetLength(), 10u);
    EXPECT_TRUE(buf.SetLength(0));
    EXPECT_EQ(buf.GetLength(), 0u);
}

TEST_F(PartiallyConsumedBufferTest, SetLength_AtCapacityFillsWriteWindow)
{
    // 从读位置到缓冲末尾的容量 = 剩余长度 + 写窗口；登记满容量后写窗口应为 0
    const size_t capacity = buffer_.GetLength() + buffer_.GetWriteBufferSize();
    ASSERT_EQ(capacity, TestBufferSize - 5);
    EXPECT_TRUE(buffer_.SetLength(capacity));
    ExpectFilledToEndOfCapacity();
}

TEST_F(PartiallyConsumedBufferTest, SetLength_ReportsLengthFromReadPosition)
{
    // 登记的是「从读位置起的有效字节数」，不搬移数据、也不是写入增量
    EXPECT_TRUE(buffer_.SetLength(3));
    EXPECT_EQ(buffer_.GetLength(), 3u);
    EXPECT_EQ(buffer_.GetWriteBufferSize(), TestBufferSize - 8);
    EXPECT_EQ(std::memcmp(buffer_.GetData(), "Wor", 3), 0);
    EXPECT_EQ(buffer_.Append("ZZ", 2), 2u);
    EXPECT_EQ(buffer_.GetLength(), 5u);
}

#if defined(GTEST_HAS_DEATH_TEST) && !defined(NDEBUG)
TEST_F(PartiallyConsumedBufferTest, SetLength_BeyondCapacityTripsAssert)
{
    // 超出读位置到缓冲末尾的距离，Debug 构建下必须由断言当场拦下
    ASSERT_DEATH(buffer_.SetLength(TestBufferSize), "len <= capacity");
}
#endif

#ifdef NDEBUG
// 断言关闭后返回值是唯一防线，故本用例只在 Release 构建下有意义
TEST_F(PartiallyConsumedBufferTest, SetLength_BeyondCapacityReportsRejectionWithoutAssert)
{
    EXPECT_FALSE(buffer_.SetLength(TestBufferSize));
    EXPECT_EQ(buffer_.GetLength(), 0u);
    EXPECT_EQ(buffer_.GetWriteBufferSize(), TestBufferSize - 5);
    EXPECT_EQ(buffer_.GetData(), buffer_.GetWritePos());
    EXPECT_EQ(std::memcmp(buffer_.GetData(), "World", 5), 0);
    EXPECT_EQ(buffer_.Append("Z", 1), 1u);
}
#endif

// ---------- GetWriteBufferSize ----------

TEST(LinearBufferTest, GetWriteBufferSize)
{
    LinearBuffer<TestBufferSize> buf;
    // Initially: full buffer available
    EXPECT_EQ(buf.GetWriteBufferSize(), TestBufferSize);

    buf.Append("Hello", 5);
    EXPECT_EQ(buf.GetWriteBufferSize(), TestBufferSize - 5);

    buf.Shift(3);
    // Length = 2, write position = ReadPos + Length = buf+3+2 = buf+5
    // WriteBufferSize = (buf + 64) - (buf + 5) = 59
    EXPECT_EQ(buf.GetWriteBufferSize(), TestBufferSize - 5);
}

// ---------- MemMove ----------

TEST(LinearBufferTest, MemMove_CompactsToFront)
{
    LinearBuffer<TestBufferSize> buf;
    // Fill, shift some, then MemMove should compact remaining to front
    buf.Append(std::string(TestBufferSize, 'A').data(), TestBufferSize);
    buf.Shift(TestBufferSize - 10); // keep last 10 bytes
    EXPECT_EQ(buf.GetLength(), 10u);

    buf.MemMove();
    EXPECT_EQ(buf.GetLength(), 10u);
    // After MemMove, data should be at the beginning of the buffer
    EXPECT_EQ(buf.GetData(), buf.GetWritePos() - 10);
}

TEST(LinearBufferTest, MemMove_EmptyBuffer)
{
    LinearBuffer<TestBufferSize> buf;
    buf.MemMove(); // should be a no-op
    EXPECT_EQ(buf.GetLength(), 0u);
}

TEST(LinearBufferTest, MemMove_EnablesMoreWrites)
{
    LinearBuffer<TestBufferSize> buf;

    // Fill, shift most, then MemMove to get write space back
    buf.Append(std::string(TestBufferSize, 'A').data(), TestBufferSize);
    buf.Shift(TestBufferSize / 2);
    buf.Append(std::string(TestBufferSize / 2, 'B').data(), TestBufferSize / 2);
    // Now the buffer is "full" in terms of write window:
    // ReadPos is at buf + 32, Length = 32, so write position is at buf + 64 = end
    EXPECT_EQ(buf.GetWriteBufferSize(), 0u);

    // But there's free space at the front (0..31) — MemMove fixes that
    buf.MemMove();
    EXPECT_GT(buf.GetWriteBufferSize(), 0u);
}

// ---------- 拷贝 / 移动 ----------

TEST(LinearBufferTest, CopyAndMoveAreDeleted)
{
    EXPECT_FALSE(std::is_copy_constructible_v<LinearBuffer<TestBufferSize>>);
    EXPECT_FALSE(std::is_copy_assignable_v<LinearBuffer<TestBufferSize>>);
    EXPECT_FALSE(std::is_move_constructible_v<LinearBuffer<TestBufferSize>>);
    EXPECT_FALSE(std::is_move_assignable_v<LinearBuffer<TestBufferSize>>);
}

// ---------- Reset ----------

TEST(LinearBufferTest, Reset)
{
    LinearBuffer<TestBufferSize> buf;
    buf.Append("Hello", 5);
    buf.Shift(2);
    buf.Reset();
    EXPECT_EQ(buf.GetLength(), 0u);
    EXPECT_EQ(buf.GetData(), buf.GetWritePos());
}

// ---------- 完整工作流 ----------

TEST(LinearBufferTest, FullWorkflow)
{
    LinearBuffer<TestBufferSize> buf;

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
