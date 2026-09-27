#include <gtest/gtest.h>
#include <Spark/TemplateLib/Buffer/ShmBuffer.h>

#include <cstring>
#include <limits>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
using namespace Spark;
// ============================================================
// ShmBuffer 测试 — 共享内存缓冲区
// 使用 std::vector 模拟共享内存，验证数据写入/读取路径
// ============================================================

static constexpr unsigned TestShmBufferSize = 256;

static_assert(std::is_same_v<decltype(std::declval<const ShmBuffer<TestShmBufferSize>&>().GetShmHeader()), const SingleShmHeader*>,
              "GetShmHeader must hand out a read-only view of the shared header");

static_assert(sizeof(SingleShmHeader) == 7 * sizeof(unsigned), "SingleShmHeader layout is pinned by ShmMappingLayoutVersion");

template <typename BufferType>
concept HasObjectPoolRoundTrip = requires(BufferType* buffer) {
    buffer->Deallocate();
    BufferType::Allocate(ServerTypeType::Client, 1, static_cast<void*>(buffer), ConnectStatusType::Connected);
};

static_assert(!HasObjectPoolRoundTrip<ShmBuffer<TestShmBufferSize>>,
              "ShmBuffer must not hand out the ObjectPool round trip from the installed header");

class ObjectPoolRoundTripProbe
{
public:
    void Deallocate() {}
    static ObjectPoolRoundTripProbe* Allocate(ServerTypeType, int, void*, ConnectStatusType) { return nullptr; }
};

static_assert(HasObjectPoolRoundTrip<ObjectPoolRoundTripProbe>, "the round-trip probe must satisfy the concept");

// 为 ShmBuffer 分配足够大的模拟内存并初始化 header
// 使用 index=1 避免 index=0 时 header 与 up_buffer 重叠
struct ShmTestFixture : public ::testing::Test
{
    void SetUp() override
    {
        // 内存布局 (index=1):
        //   [0 .. sizeof(SingleShmHeader))  — header (index=0)
        //   [sizeof(SingleShmHeader) .. 2*Size) — 填充
        //   [2*Size .. 3*Size)              — UpBuffer
        //   [3*Size .. 4*Size)              — DownBuffer
        memory_.resize(sizeof(SingleShmHeader) + TestShmBufferSize * 4, 0);

        // 使用 index=1 构造 — ShmBuffer 自行定位 header、UpBuffer、DownBuffer
        client_.reset(new ShmBuffer<TestShmBufferSize>(ServerTypeType::Client, 1, memory_.data(), ConnectStatusType::Connected));
        server_.reset(new ShmBuffer<TestShmBufferSize>(ServerTypeType::Server, 1, memory_.data(), ConnectStatusType::Connected));

        header_ = client_->GetShmHeader();
    }

    void TearDown() override
    {
        client_.reset();
        server_.reset();
    }

    const SingleShmHeader* HeaderOfSecondConnection() const { return reinterpret_cast<const SingleShmHeader*>(memory_.data()) + 1; }

    std::vector<char> memory_;
    const SingleShmHeader* header_ = nullptr;
    std::unique_ptr<ShmBuffer<TestShmBufferSize>> client_;
    std::unique_ptr<ShmBuffer<TestShmBufferSize>> server_;
};

// ========== 构造 ==========

TEST(ShmBufferTest, StatusConnected)
{
    std::vector<char> memory(sizeof(SingleShmHeader) + TestShmBufferSize * 4, 0);
    ShmBuffer<TestShmBufferSize> buffer(ServerTypeType::Server, 1, memory.data(), ConnectStatusType::Connected);
    EXPECT_EQ(buffer.GetConnectStatus(), ConnectStatusType::Connected);
}

TEST(ShmBufferTest, GetConnectStatusIsUnConnectedOnDefaultConstructedBuffer)
{
    ShmBuffer<TestShmBufferSize> buf;
    EXPECT_EQ(buf.GetShmHeader(), nullptr);
    EXPECT_EQ(buf.GetConnectStatus(), ConnectStatusType::UnConnected);
}

// 未 Attach 的实例（shmHeader_ 为空）不得触碰任何共享内存：入口一律惰性返回，写状态一律不生效
TEST(ShmBufferTest, UnAttachedBufferRejectsAllChannelAccess)
{
    ShmBuffer<TestShmBufferSize> buffer;
    char byte = 0;

    EXPECT_EQ(buffer.Write(&byte, 1), 0u);
    EXPECT_EQ(buffer.Read(&byte, 1), 0u);
    EXPECT_EQ(buffer.GetWriteBufferSize(), 0u);
    EXPECT_EQ(buffer.GetReadBufferSize(), 0u);
    EXPECT_FALSE(buffer.MarkDisconnectedAndReportWhetherLastHolder());
    buffer.SetConnectStatus(ConnectStatusType::Connected);
    buffer.ResetSharedHeader();
    EXPECT_EQ(buffer.GetShmHeader(), nullptr);
    EXPECT_EQ(buffer.GetConnectStatus(), ConnectStatusType::UnConnected);
}

#if defined(GTEST_HAS_DEATH_TEST) && !defined(NDEBUG)
TEST(ShmBufferTest, ConnectionIndexZeroTripsAssert)
{
    // index=0 时 UpBuffer 会落在 header 区上，Debug 构建下必须由断言当场拦下
    std::vector<char> memory(sizeof(SingleShmHeader) + TestShmBufferSize * 4, 0);
    ASSERT_DEATH(
        {
            ShmBuffer<TestShmBufferSize> buffer(ServerTypeType::Client, 0, memory.data(), ConnectStatusType::Connected);
            (void)buffer;
        },
        "IsValidConnectionIndex");
}
#endif

#ifdef NDEBUG
// 断言关闭后非法构造不再被拦下，改为「保持未 Attach」：此后通道访问与状态写入一律惰性返回，共享内存一个字节都不动
TEST(ShmBufferTest, InvalidConnectionIndexLeavesBufferDetached)
{
    std::vector<char> memory(sizeof(SingleShmHeader) + TestShmBufferSize * 4, 0);
    ShmBuffer<TestShmBufferSize> zeroIndexed(ServerTypeType::Client, 0, memory.data(), ConnectStatusType::Connected);
    ShmBuffer<TestShmBufferSize> negativeIndexed(ServerTypeType::Client, -1, memory.data(), ConnectStatusType::Connected);

    EXPECT_EQ(zeroIndexed.GetShmHeader(), nullptr);
    EXPECT_EQ(zeroIndexed.GetConnectStatus(), ConnectStatusType::UnConnected);
    EXPECT_EQ(zeroIndexed.Write("data", 4), 0u);
    EXPECT_EQ(zeroIndexed.GetWriteBufferSize(), 0u);
    EXPECT_EQ(reinterpret_cast<const SingleShmHeader*>(memory.data())->Status, ConnectStatusType::UnConnected);
    EXPECT_EQ(negativeIndexed.GetShmHeader(), nullptr);
    EXPECT_EQ(negativeIndexed.GetWriteBufferSize(), 0u);
}

TEST(ShmBufferTest, NullSharedMemoryBaseLeavesBufferDetached)
{
    ShmBuffer<TestShmBufferSize> buffer(ServerTypeType::Server, 1, nullptr, ConnectStatusType::Connected);

    EXPECT_EQ(buffer.GetShmHeader(), nullptr);
    EXPECT_EQ(buffer.GetConnectStatus(), ConnectStatusType::UnConnected);
    EXPECT_EQ(buffer.Write("data", 4), 0u);
    EXPECT_EQ(buffer.GetReadBufferSize(), 0u);
}
#endif

TEST(ShmBufferTest, ConnectionIndexWithinMappingRequiresBothBounds)
{
    using Buffer = ShmBuffer<TestShmBufferSize>;

    EXPECT_TRUE(Buffer::IsConnectionIndexWithinMapping(1, 4));
    EXPECT_TRUE(Buffer::IsConnectionIndexWithinMapping(3, 4));
    EXPECT_FALSE(Buffer::IsConnectionIndexWithinMapping(0, 4));
    EXPECT_FALSE(Buffer::IsConnectionIndexWithinMapping(4, 4));
    EXPECT_FALSE(Buffer::IsConnectionIndexWithinMapping((std::numeric_limits<unsigned>::max)(), 4));
    EXPECT_FALSE(Buffer::IsConnectionIndexWithinMapping(1, 1));
}

// ========== Client 写（UpWrite）/ 读（DownRead）==========

TEST_F(ShmTestFixture, ClientWrite_UpBuffer)
{
    const char* data = "HelloShm";
    size_t written = client_->Write(data, 8);
    EXPECT_EQ(written, 8u);

    // Client Write → UpWrite → data in UpBuffer
    EXPECT_EQ(std::memcmp(memory_.data() + TestShmBufferSize * 2, data, 8), 0);
    EXPECT_EQ(header_->UpWriteCount, 8u);
}

TEST_F(ShmTestFixture, ClientRead_DownBuffer)
{
    // 先通过 Server 往 DownBuffer 写入数据 (Server → DownWrite)
    const char* data = "FromServer";
    server_->Write(data, 10);
    EXPECT_EQ(header_->DownWriteCount, 10u);

    // Client 读取 DownBuffer (Client → DownRead)
    char output[32] = {};
    size_t read = client_->Read(output, 10);
    EXPECT_EQ(read, 10u);
    EXPECT_EQ(std::memcmp(output, "FromServer", 10), 0);
    EXPECT_EQ(header_->DownReadCount, 10u);
}

// ========== Server 写（DownWrite）/ 读（UpRead）==========

TEST_F(ShmTestFixture, ServerWrite_DownBuffer)
{
    const char* data = "ServerData";
    size_t written = server_->Write(data, 10);
    EXPECT_EQ(written, 10u);

    // Server Write → DownWrite → data in DownBuffer
    EXPECT_EQ(std::memcmp(memory_.data() + TestShmBufferSize * 3, data, 10), 0);
    EXPECT_EQ(header_->DownWriteCount, 10u);
}

TEST_F(ShmTestFixture, ServerRead_UpBuffer)
{
    // 先通过 Client 往 UpBuffer 写入数据 (Client → UpWrite)
    const char* data = "FromClient";
    client_->Write(data, 10);
    EXPECT_EQ(header_->UpWriteCount, 10u);

    // Server 读取 UpBuffer (Server → UpRead)
    char output[32] = {};
    size_t read = server_->Read(output, 10);
    EXPECT_EQ(read, 10u);
    EXPECT_EQ(std::memcmp(output, "FromClient", 10), 0);
    EXPECT_EQ(header_->UpReadCount, 10u);
}

// ========== 断连状态 ==========

TEST_F(ShmTestFixture, WriteWhenDisconnected_ReturnsZero)
{
    client_->SetConnectStatus(ConnectStatusType::DisConnected);
    EXPECT_EQ(client_->Write("data", 4), 0u);
    EXPECT_EQ(server_->Write("data", 4), 0u);
}

TEST_F(ShmTestFixture, ReadWhenDisconnected_ReturnsZero)
{
    client_->SetConnectStatus(ConnectStatusType::DisConnected);
    char buf[16] = {};
    EXPECT_EQ(client_->Read(buf, 4), 0u);
    EXPECT_EQ(server_->Read(buf, 4), 0u);
}

// ========== 缓冲区大小查询 ==========

TEST_F(ShmTestFixture, GetWriteBufferSize_Client)
{
    // Client WriteBufferSize = GetUpWriteBufferSize
    // 初始: Size - 0 - 1 = TestShmBufferSize - 1
    EXPECT_EQ(client_->GetWriteBufferSize(), TestShmBufferSize - 1);

    client_->Write("Hello", 5);
    // UpWriteCount = 5, UpReadCount = 0
    // GetUpWriteBufferSize: Read > Write? no, so Size - (5 - 0) - 1 = Size - 6
    EXPECT_EQ(client_->GetWriteBufferSize(), TestShmBufferSize - 6);
}

TEST_F(ShmTestFixture, GetReadBufferSize_Client)
{
    // Client ReadBufferSize = GetDownReadBufferSize
    EXPECT_EQ(client_->GetReadBufferSize(), 0u);

    server_->Write("Data", 4);
    EXPECT_EQ(client_->GetReadBufferSize(), 4u);
}

TEST_F(ShmTestFixture, GetWriteBufferSize_Server)
{
    EXPECT_EQ(server_->GetWriteBufferSize(), TestShmBufferSize - 1);

    server_->Write("Hello", 5);
    EXPECT_EQ(server_->GetWriteBufferSize(), TestShmBufferSize - 6);
}

TEST_F(ShmTestFixture, GetReadBufferSize_Server)
{
    EXPECT_EQ(server_->GetReadBufferSize(), 0u);

    client_->Write("Data", 4);
    EXPECT_EQ(server_->GetReadBufferSize(), 4u);
}

// ========== 容量边界 ==========

TEST_F(ShmTestFixture, WriteWhenFull_ReturnsZero)
{
    std::string filling(TestShmBufferSize - 1, 'A');
    EXPECT_EQ(client_->Write(filling.data(), TestShmBufferSize - 1), TestShmBufferSize - 1);
    EXPECT_EQ(client_->GetWriteBufferSize(), 0u);

    EXPECT_EQ(client_->Write("X", 1), 0u);
    EXPECT_EQ(header_->UpWriteCount, TestShmBufferSize - 1);
}

TEST_F(ShmTestFixture, Write_TruncatesToFreeSpace)
{
    std::string filling(TestShmBufferSize - 10, 'A');
    client_->Write(filling.data(), TestShmBufferSize - 10);
    EXPECT_EQ(client_->GetWriteBufferSize(), 9u);

    std::string overflow(20, 'B');
    EXPECT_EQ(client_->Write(overflow.data(), 20), 9u);
    EXPECT_EQ(header_->UpWriteCount, TestShmBufferSize - 1);
}

TEST_F(ShmTestFixture, Read_TruncatesToAvailable)
{
    client_->Write("Hello", 5);

    char output[16] = {};
    EXPECT_EQ(server_->Read(output, 16), 5u);
    EXPECT_EQ(std::memcmp(output, "Hello", 5), 0);
    EXPECT_EQ(header_->UpReadCount, 5u);
}

TEST_F(ShmTestFixture, ReadWhenEmpty_ReturnsZero)
{
    char output[16] = {};
    EXPECT_EQ(server_->Read(output, 16), 0u);
    EXPECT_EQ(client_->Read(output, 16), 0u);
}

// ========== 绕回 wrap-around ==========

TEST_F(ShmTestFixture, Write_WrapsAround)
{
    // Step 1: 写 250 字节填满到接近末尾
    std::string first(250, 'A');
    client_->Write(first.data(), 250);
    EXPECT_EQ(header_->UpWriteCount, 250u);

    // Step 2: Server 读取 100 字节，使 UpReadCount 前进以释放槽位
    char temp[128] = {};
    server_->Read(temp, 100);
    EXPECT_EQ(header_->UpReadCount, 100u);

    // Step 3: 写 105 字节 — 尾部 6 字节 (250→256) + 头部 99 字节 (0→99) 绕回
    //   GetUpWriteBufferSize() = 256 - (250-100) - 1 = 105
    std::string second(105, 'B');
    size_t written = client_->Write(second.data(), 105);
    EXPECT_EQ(written, 105u);
    EXPECT_EQ(header_->UpWriteCount, 99u); // 105 - 6 = 99 (绕回值)

    // 此刻 UpWriteCount(99) < UpReadCount(100)，走 CountWritableBytes 的绕回分支：100 - 99 - 1 = 0
    EXPECT_EQ(client_->GetWriteBufferSize(), 0u);

    // Step 4: Server 读取全部数据
    //   内存布局: [0..99)=B, [100..250)=A, [250..256)=B
    char output[TestShmBufferSize] = {};
    size_t read = server_->Read(output, TestShmBufferSize);
    EXPECT_EQ(read, 255u);

    // 前 150 字节是为读取的 'A' (从位置 100 到 249)
    for (unsigned i = 0; i < 150; ++i)
        EXPECT_EQ(output[i], 'A');
    // 后 105 字节为 'B' (尾部 6 字节 + 头部 99 字节)
    for (unsigned i = 150; i < 255; ++i)
        EXPECT_EQ(output[i], 'B');
}

TEST_F(ShmTestFixture, DownChannel_WrapsAround)
{
    std::string first(250, 'A');
    server_->Write(first.data(), 250);
    EXPECT_EQ(header_->DownWriteCount, 250u);

    char temp[128] = {};
    client_->Read(temp, 100);
    EXPECT_EQ(header_->DownReadCount, 100u);

    std::string second(105, 'B');
    EXPECT_EQ(server_->Write(second.data(), 105), 105u);
    EXPECT_EQ(header_->DownWriteCount, 99u);
    EXPECT_EQ(server_->GetWriteBufferSize(), 0u);

    char output[TestShmBufferSize] = {};
    EXPECT_EQ(client_->Read(output, TestShmBufferSize), 255u);

    for (unsigned i = 0; i < 150; ++i)
        EXPECT_EQ(output[i], 'A');
    for (unsigned i = 150; i < 255; ++i)
        EXPECT_EQ(output[i], 'B');
}

// ========== 计数器达到通道容量 Size ==========

TEST_F(ShmTestFixture, CountersReachChannelSizeExactly)
{
    // 写指针恰好停在 Size 上：writeIndex 不取模，而是让 writeCount 直接等于 Size
    std::string head(TestShmBufferSize - 6, 'A');
    EXPECT_EQ(client_->Write(head.data(), TestShmBufferSize - 6), TestShmBufferSize - 6);
    EXPECT_EQ(header_->UpWriteCount, TestShmBufferSize - 6);

    char drained[TestShmBufferSize] = {};
    EXPECT_EQ(server_->Read(drained, TestShmBufferSize - 6), TestShmBufferSize - 6);
    EXPECT_EQ(header_->UpReadCount, TestShmBufferSize - 6);

    // 尾部正好剩 6 字节，本次写入不绕回，writeCount 落在 Size 上
    EXPECT_EQ(client_->Write("BBBBBB", 6), 6u);
    EXPECT_EQ(header_->UpWriteCount, TestShmBufferSize);
    EXPECT_EQ(server_->GetReadBufferSize(), 6u);
    EXPECT_EQ(client_->GetWriteBufferSize(), TestShmBufferSize - 7);

    char tail[8] = {};
    EXPECT_EQ(server_->Read(tail, 6), 6u);
    EXPECT_EQ(header_->UpReadCount, TestShmBufferSize);

    // writeIndex == Size 时头部拷贝长度为 0，写入必须整段落进绕回分支
    EXPECT_EQ(client_->Write("C", 1), 1u);
    EXPECT_EQ(header_->UpWriteCount, 1u);

    char wrapped[4] = {};
    EXPECT_EQ(server_->Read(wrapped, 1), 1u);
    EXPECT_EQ(wrapped[0], 'C');
    EXPECT_EQ(header_->UpReadCount, 1u);
}

// ========== 多次 Write / Read 周期 ==========

TEST_F(ShmTestFixture, MultiCycle)
{
    for (int i = 0; i < 5; ++i)
    {
        std::string msg = "Msg" + std::to_string(i);
        client_->Write(msg.data(), msg.size());
    }
    EXPECT_EQ(header_->UpWriteCount, 20u); // "Msg0".."Msg4" each 4 bytes

    for (int i = 0; i < 5; ++i)
    {
        char output[16] = {};
        server_->Read(output, 4);
        std::string expected = "Msg" + std::to_string(i);
        EXPECT_EQ(std::memcmp(output, expected.data(), 4), 0);
    }
    EXPECT_EQ(header_->UpReadCount, 20u);
}

// ========== 共享头状态机 ==========

TEST_F(ShmTestFixture, MarkDisconnectedAndReportWhetherLastHolder_ReportsLastOwner)
{
    EXPECT_FALSE(client_->MarkDisconnectedAndReportWhetherLastHolder());
    EXPECT_EQ(header_->Status, ConnectStatusType::DisConnected);
    EXPECT_TRUE(client_->MarkDisconnectedAndReportWhetherLastHolder());
    EXPECT_EQ(header_->Status, ConnectStatusType::DisConnected);
}

TEST_F(ShmTestFixture, ResetSharedHeader_ClearsCountersAndStatus)
{
    client_->Write("Hello", 5);
    server_->Write("World", 5);
    EXPECT_EQ(header_->UpWriteCount, 5u);
    EXPECT_EQ(header_->DownWriteCount, 5u);

    client_->ResetSharedHeader();

    EXPECT_EQ(header_->Status, ConnectStatusType::UnConnected);
    EXPECT_EQ(header_->UpWriteCount, 0u);
    EXPECT_EQ(header_->UpReadCount, 0u);
    EXPECT_EQ(header_->DownWriteCount, 0u);
    EXPECT_EQ(header_->DownReadCount, 0u);
}

// ========== 退让共享通道 ==========

TEST_F(ShmTestFixture, MarkDisconnectedAndResetChannelWhenLastHolder_MarksDisconnectedWhenStillConnected)
{
    ShmBuffer<TestShmBufferSize> buffer(ServerTypeType::Client, 1, memory_.data(), ConnectStatusType::Connected);

    buffer.MarkDisconnectedAndResetChannelWhenLastHolder();

    EXPECT_EQ(HeaderOfSecondConnection()->Status, ConnectStatusType::DisConnected);
}

TEST_F(ShmTestFixture, MarkDisconnectedAndResetChannelWhenLastHolder_ResetsHeaderWhenAlreadyDisconnected)
{
    ShmBuffer<TestShmBufferSize> buffer(ServerTypeType::Client, 1, memory_.data(), ConnectStatusType::Connected);
    EXPECT_EQ(buffer.Write("Hello", 5), 5u);
    EXPECT_FALSE(buffer.MarkDisconnectedAndReportWhetherLastHolder());

    buffer.MarkDisconnectedAndResetChannelWhenLastHolder();

    EXPECT_EQ(HeaderOfSecondConnection()->Status, ConnectStatusType::UnConnected);
    EXPECT_EQ(HeaderOfSecondConnection()->UpWriteCount, 0u);
}
