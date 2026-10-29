#include "AssertAbortProbe.h"
#include <Spark/TemplateLib/Buffer/ShmBuffer.h>
#include "doctest/doctest.h"

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
static constexpr unsigned TestShmConnectCount = 2;
static constexpr int OutOfRangeConnectionIndex = static_cast<int>(TestShmConnectCount);

// 模拟映射内存：index=1 的通道头与上下行缓冲都要落在这块内存里
std::vector<char> MakeZeroedShmMapping()
{
    return std::vector<char>(sizeof(SingleShmHeader) + TestShmBufferSize * 4, 0);
}

static_assert(std::is_same_v<decltype(std::declval<const ShmBuffer<TestShmBufferSize>&>().GetShmHeader()), const SingleShmHeader*>,
              "GetShmHeader must hand out a read-only view of the shared header");

static_assert(sizeof(SingleShmHeader) == 8 + 6 * sizeof(size_t), "SingleShmHeader layout is pinned by ShmMappingLayoutVersion");

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
struct ShmTestFixture
{
    ShmTestFixture()
    {
        // 内存布局 (index=1):
        //   [0 .. sizeof(SingleShmHeader))  — header (index=0)
        //   [sizeof(SingleShmHeader) .. 2*Size) — 填充
        //   [2*Size .. 3*Size)              — UpBuffer
        //   [3*Size .. 4*Size)              — DownBuffer
        memory_ = MakeZeroedShmMapping();

        // 使用 index=1 构造 — ShmBuffer 自行定位 header、UpBuffer、DownBuffer
        client_.reset(new ShmBuffer<TestShmBufferSize>(ServerTypeType::Client, 1, memory_.data(), ConnectStatusType::Connected, TestShmConnectCount));
        server_.reset(new ShmBuffer<TestShmBufferSize>(ServerTypeType::Server, 1, memory_.data(), ConnectStatusType::Connected, TestShmConnectCount));

        header_ = client_->GetShmHeader();
    }

    ~ShmTestFixture()
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

TEST_CASE("ShmBufferTest.StatusConnected")
{
    auto memory = MakeZeroedShmMapping();
    ShmBuffer<TestShmBufferSize> buffer(ServerTypeType::Server, 1, memory.data(), ConnectStatusType::Connected, TestShmConnectCount);
    CHECK_EQ(buffer.GetConnectStatus(), ConnectStatusType::Connected);
}

TEST_CASE("ShmBufferTest.GetConnectStatusIsUnConnectedOnDefaultConstructedBuffer")
{
    ShmBuffer<TestShmBufferSize> buf;
    CHECK_EQ(buf.GetShmHeader(), nullptr);
    CHECK_EQ(buf.GetConnectStatus(), ConnectStatusType::UnConnected);
}

// 未 Attach 的实例（shmHeader_ 为空）不得触碰任何共享内存：入口一律惰性返回，写状态一律不生效
TEST_CASE("ShmBufferTest.UnAttachedBufferRejectsAllChannelAccess")
{
    ShmBuffer<TestShmBufferSize> buffer;
    char byte = 0;

    CHECK_EQ(buffer.Write(&byte, 1), 0u);
    CHECK_EQ(buffer.Read(&byte, 1), 0u);
    CHECK_EQ(buffer.GetWriteBufferSize(), 0u);
    CHECK_EQ(buffer.GetReadBufferSize(), 0u);
    CHECK_FALSE(buffer.MarkDisconnectedAndReportWhetherLastHolder());
    CHECK_FALSE(buffer.RevokeUnconfirmedAccept());
    buffer.SetConnectStatus(ConnectStatusType::Connected);
    buffer.ResetSharedHeader();
    CHECK_EQ(buffer.GetShmHeader(), nullptr);
    CHECK_EQ(buffer.GetConnectStatus(), ConnectStatusType::UnConnected);
}

#ifndef NDEBUG
TEST_CASE("ShmBufferTest.OutOfRangeConnectionIndexTripsAssert")
{
    // index=0 时 UpBuffer 会落在 header 区上，index 到达容量时 header 已越出映射，Debug 构建下必须由断言当场拦下；
    // 断言在子进程里触发，父进程核对退出方式与断言消息
    CHECK(Spark::Test::ObserveAssertAbortInChildProcess("ShmBufferZeroConnectionIndex", "IsValidConnectionIndex"));
    CHECK(Spark::Test::ObserveAssertAbortInChildProcess("ShmBufferOutOfRangeConnectionIndex", "IsValidConnectionIndex"));
}
#endif

#ifdef NDEBUG
// 断言关闭后非法构造不再被拦下，改为「保持未 Attach」：此后通道访问与状态写入一律惰性返回，共享内存一个字节都不动
TEST_CASE("ShmBufferTest.InvalidConnectionIndexLeavesBufferDetached")
{
    auto memory = MakeZeroedShmMapping();
    ShmBuffer<TestShmBufferSize> zeroIndexed(ServerTypeType::Client, 0, memory.data(), ConnectStatusType::Connected, TestShmConnectCount);
    ShmBuffer<TestShmBufferSize> negativeIndexed(ServerTypeType::Client, -1, memory.data(), ConnectStatusType::Connected, TestShmConnectCount);
    ShmBuffer<TestShmBufferSize> beyondCapacityIndexed(ServerTypeType::Client, OutOfRangeConnectionIndex, memory.data(), ConnectStatusType::Connected,
                                                       TestShmConnectCount);

    CHECK_EQ(zeroIndexed.GetShmHeader(), nullptr);
    CHECK_EQ(zeroIndexed.GetConnectStatus(), ConnectStatusType::UnConnected);
    CHECK_EQ(zeroIndexed.Write("data", 4), 0u);
    CHECK_EQ(zeroIndexed.GetWriteBufferSize(), 0u);
    CHECK_EQ(reinterpret_cast<const SingleShmHeader*>(memory.data())->Status, ConnectStatusType::UnConnected);
    CHECK_EQ(negativeIndexed.GetShmHeader(), nullptr);
    CHECK_EQ(negativeIndexed.GetWriteBufferSize(), 0u);
    CHECK_EQ(beyondCapacityIndexed.GetShmHeader(), nullptr);
    CHECK_EQ(beyondCapacityIndexed.GetConnectStatus(), ConnectStatusType::UnConnected);
    CHECK_EQ(beyondCapacityIndexed.GetReadBufferSize(), 0u);
}

TEST_CASE("ShmBufferTest.NullSharedMemoryBaseLeavesBufferDetached")
{
    ShmBuffer<TestShmBufferSize> buffer(ServerTypeType::Server, 1, nullptr, ConnectStatusType::Connected, TestShmConnectCount);

    CHECK_EQ(buffer.GetShmHeader(), nullptr);
    CHECK_EQ(buffer.GetConnectStatus(), ConnectStatusType::UnConnected);
    CHECK_EQ(buffer.Write("data", 4), 0u);
    CHECK_EQ(buffer.GetReadBufferSize(), 0u);
}
#endif

TEST_CASE("ShmBufferTest.ConnectionIndexWithinMappingRequiresBothBounds")
{
    using Buffer = ShmBuffer<TestShmBufferSize>;

    CHECK(Buffer::IsConnectionIndexWithinMapping(1, 4));
    CHECK(Buffer::IsConnectionIndexWithinMapping(3, 4));
    CHECK_FALSE(Buffer::IsConnectionIndexWithinMapping(0, 4));
    CHECK_FALSE(Buffer::IsConnectionIndexWithinMapping(4, 4));
    CHECK_FALSE(Buffer::IsConnectionIndexWithinMapping((std::numeric_limits<unsigned>::max)(), 4));
    CHECK_FALSE(Buffer::IsConnectionIndexWithinMapping(1, 1));
}

// ========== Client 写（UpWrite）/ 读（DownRead）==========

TEST_CASE_FIXTURE(ShmTestFixture, "ShmTestFixture.ClientWrite_UpBuffer")
{
    const char* data = "HelloShm";
    size_t written = client_->Write(data, 8);
    CHECK_EQ(written, 8u);

    // Client Write → UpWrite → data in UpBuffer
    CHECK_EQ(std::memcmp(memory_.data() + TestShmBufferSize * 2, data, 8), 0);
    CHECK_EQ(header_->UpWriteCount, 8u);
}

TEST_CASE_FIXTURE(ShmTestFixture, "ShmTestFixture.ClientRead_DownBuffer")
{
    // 先通过 Server 往 DownBuffer 写入数据 (Server → DownWrite)
    const char* data = "FromServer";
    server_->Write(data, 10);
    CHECK_EQ(header_->DownWriteCount, 10u);

    // Client 读取 DownBuffer (Client → DownRead)
    char output[32] = {};
    size_t read = client_->Read(output, 10);
    CHECK_EQ(read, 10u);
    CHECK_EQ(std::memcmp(output, "FromServer", 10), 0);
    CHECK_EQ(header_->DownReadCount, 10u);
}

// ========== Server 写（DownWrite）/ 读（UpRead）==========

TEST_CASE_FIXTURE(ShmTestFixture, "ShmTestFixture.ServerWrite_DownBuffer")
{
    const char* data = "ServerData";
    size_t written = server_->Write(data, 10);
    CHECK_EQ(written, 10u);

    // Server Write → DownWrite → data in DownBuffer
    CHECK_EQ(std::memcmp(memory_.data() + TestShmBufferSize * 3, data, 10), 0);
    CHECK_EQ(header_->DownWriteCount, 10u);
}

TEST_CASE_FIXTURE(ShmTestFixture, "ShmTestFixture.ServerRead_UpBuffer")
{
    // 先通过 Client 往 UpBuffer 写入数据 (Client → UpWrite)
    const char* data = "FromClient";
    client_->Write(data, 10);
    CHECK_EQ(header_->UpWriteCount, 10u);

    // Server 读取 UpBuffer (Server → UpRead)
    char output[32] = {};
    size_t read = server_->Read(output, 10);
    CHECK_EQ(read, 10u);
    CHECK_EQ(std::memcmp(output, "FromClient", 10), 0);
    CHECK_EQ(header_->UpReadCount, 10u);
}

// ========== 断连状态 ==========

TEST_CASE_FIXTURE(ShmTestFixture, "ShmTestFixture.WriteWhenDisconnected_ReturnsZero")
{
    client_->SetConnectStatus(ConnectStatusType::DisConnected);
    CHECK_EQ(client_->Write("data", 4), 0u);
    CHECK_EQ(server_->Write("data", 4), 0u);
}

TEST_CASE_FIXTURE(ShmTestFixture, "ShmTestFixture.ReadWhenDisconnected_ReturnsZero")
{
    client_->SetConnectStatus(ConnectStatusType::DisConnected);
    char buf[16] = {};
    CHECK_EQ(client_->Read(buf, 4), 0u);
    CHECK_EQ(server_->Read(buf, 4), 0u);
}

// ========== 缓冲区大小查询 ==========

TEST_CASE_FIXTURE(ShmTestFixture, "ShmTestFixture.GetWriteBufferSize_Client")
{
    CHECK_EQ(client_->GetWriteBufferSize(), TestShmBufferSize);

    client_->Write("Hello", 5);
    CHECK_EQ(client_->GetWriteBufferSize(), TestShmBufferSize - 5);
}

TEST_CASE_FIXTURE(ShmTestFixture, "ShmTestFixture.GetReadBufferSize_Client")
{
    // Client ReadBufferSize = GetDownReadBufferSize
    CHECK_EQ(client_->GetReadBufferSize(), 0u);

    server_->Write("Data", 4);
    CHECK_EQ(client_->GetReadBufferSize(), 4u);
}

TEST_CASE_FIXTURE(ShmTestFixture, "ShmTestFixture.GetWriteBufferSize_Server")
{
    CHECK_EQ(server_->GetWriteBufferSize(), TestShmBufferSize);

    server_->Write("Hello", 5);
    CHECK_EQ(server_->GetWriteBufferSize(), TestShmBufferSize - 5);
}

TEST_CASE_FIXTURE(ShmTestFixture, "ShmTestFixture.GetReadBufferSize_Server")
{
    CHECK_EQ(server_->GetReadBufferSize(), 0u);

    client_->Write("Data", 4);
    CHECK_EQ(server_->GetReadBufferSize(), 4u);
}

// ========== 容量边界 ==========

TEST_CASE_FIXTURE(ShmTestFixture, "ShmTestFixture.WriteWhenFull_ReturnsZero")
{
    std::string filling(TestShmBufferSize, 'A');
    CHECK_EQ(client_->Write(filling.data(), TestShmBufferSize), TestShmBufferSize);
    CHECK_EQ(client_->GetWriteBufferSize(), 0u);

    CHECK_EQ(client_->Write("X", 1), 0u);
    CHECK_EQ(header_->UpWriteCount, TestShmBufferSize);
}

TEST_CASE_FIXTURE(ShmTestFixture, "ShmTestFixture.Write_TruncatesToFreeSpace")
{
    std::string filling(TestShmBufferSize - 10, 'A');
    client_->Write(filling.data(), TestShmBufferSize - 10);
    CHECK_EQ(client_->GetWriteBufferSize(), 10u);

    std::string overflow(20, 'B');
    CHECK_EQ(client_->Write(overflow.data(), 20), 10u);
    CHECK_EQ(header_->UpWriteCount, TestShmBufferSize);
}

TEST_CASE_FIXTURE(ShmTestFixture, "ShmTestFixture.Read_TruncatesToAvailable")
{
    client_->Write("Hello", 5);

    char output[16] = {};
    CHECK_EQ(server_->Read(output, 16), 5u);
    CHECK_EQ(std::memcmp(output, "Hello", 5), 0);
    CHECK_EQ(header_->UpReadCount, 5u);
}

TEST_CASE_FIXTURE(ShmTestFixture, "ShmTestFixture.ReadWhenEmpty_ReturnsZero")
{
    char output[16] = {};
    CHECK_EQ(server_->Read(output, 16), 0u);
    CHECK_EQ(client_->Read(output, 16), 0u);
}

// ========== 绕回 wrap-around ==========

TEST_CASE_FIXTURE(ShmTestFixture, "ShmTestFixture.Write_WrapsAround")
{
    // Step 1: 写 250 字节填满到接近末尾
    std::string first(250, 'A');
    client_->Write(first.data(), 250);
    CHECK_EQ(header_->UpWriteCount, 250u);

    // Step 2: Server 读取 100 字节，使 UpReadCount 前进以释放槽位
    char temp[128] = {};
    server_->Read(temp, 100);
    CHECK_EQ(header_->UpReadCount, 100u);

    // Step 3: 写 105 字节 — 尾部 6 字节 (250→256) + 头部 99 字节 (0→99) 绕回
    std::string second(105, 'B');
    size_t written = client_->Write(second.data(), 105);
    CHECK_EQ(written, 105u);
    CHECK_EQ(header_->UpWriteCount, 355u);

    CHECK_EQ(client_->GetWriteBufferSize(), 1u);

    // Step 4: Server 读取全部数据
    //   内存布局: [0..99)=B, [100..250)=A, [250..256)=B
    char output[TestShmBufferSize] = {};
    size_t read = server_->Read(output, TestShmBufferSize);
    CHECK_EQ(read, 255u);

    // 前 150 字节是为读取的 'A' (从位置 100 到 249)
    for (unsigned i = 0; i < 150; ++i)
        CHECK_EQ(output[i], 'A');
    // 后 105 字节为 'B' (尾部 6 字节 + 头部 99 字节)
    for (unsigned i = 150; i < 255; ++i)
        CHECK_EQ(output[i], 'B');
}

TEST_CASE_FIXTURE(ShmTestFixture, "ShmTestFixture.DownChannel_WrapsAround")
{
    std::string first(250, 'A');
    server_->Write(first.data(), 250);
    CHECK_EQ(header_->DownWriteCount, 250u);

    char temp[128] = {};
    client_->Read(temp, 100);
    CHECK_EQ(header_->DownReadCount, 100u);

    std::string second(105, 'B');
    CHECK_EQ(server_->Write(second.data(), 105), 105u);
    CHECK_EQ(header_->DownWriteCount, 355u);
    CHECK_EQ(server_->GetWriteBufferSize(), 1u);

    char output[TestShmBufferSize] = {};
    CHECK_EQ(client_->Read(output, TestShmBufferSize), 255u);

    for (unsigned i = 0; i < 150; ++i)
        CHECK_EQ(output[i], 'A');
    for (unsigned i = 150; i < 255; ++i)
        CHECK_EQ(output[i], 'B');
}

// ========== 计数器越过通道容量 Size ==========

TEST_CASE_FIXTURE(ShmTestFixture, "ShmTestFixture.CountersGrowPastChannelSizeAndPositionWrapsByMask")
{
    std::string head(TestShmBufferSize - 6, 'A');
    CHECK_EQ(client_->Write(head.data(), TestShmBufferSize - 6), TestShmBufferSize - 6);
    CHECK_EQ(header_->UpWriteCount, TestShmBufferSize - 6);

    char drained[TestShmBufferSize] = {};
    CHECK_EQ(server_->Read(drained, TestShmBufferSize - 6), TestShmBufferSize - 6);
    CHECK_EQ(header_->UpReadCount, TestShmBufferSize - 6);

    CHECK_EQ(client_->Write("BBBBBB", 6), 6u);
    CHECK_EQ(header_->UpWriteCount, TestShmBufferSize);
    CHECK_EQ(server_->GetReadBufferSize(), 6u);
    CHECK_EQ(client_->GetWriteBufferSize(), TestShmBufferSize - 6);

    char tail[8] = {};
    CHECK_EQ(server_->Read(tail, 6), 6u);
    CHECK_EQ(header_->UpReadCount, TestShmBufferSize);

    CHECK_EQ(client_->Write("C", 1), 1u);
    CHECK_EQ(header_->UpWriteCount, TestShmBufferSize + 1);
    CHECK_EQ(client_->GetWriteBufferSize(), TestShmBufferSize - 1);

    char wrapped[4] = {};
    CHECK_EQ(server_->Read(wrapped, 1), 1u);
    CHECK_EQ(wrapped[0], 'C');
    CHECK_EQ(header_->UpReadCount, TestShmBufferSize + 1);
}

// ========== 多次 Write / Read 周期 ==========

TEST_CASE_FIXTURE(ShmTestFixture, "ShmTestFixture.MultiCycle")
{
    for (int i = 0; i < 5; ++i)
    {
        std::string msg = "Msg" + std::to_string(i);
        client_->Write(msg.data(), msg.size());
    }
    CHECK_EQ(header_->UpWriteCount, 20u); // "Msg0".."Msg4" each 4 bytes

    for (int i = 0; i < 5; ++i)
    {
        char output[16] = {};
        server_->Read(output, 4);
        std::string expected = "Msg" + std::to_string(i);
        CHECK_EQ(std::memcmp(output, expected.data(), 4), 0);
    }
    CHECK_EQ(header_->UpReadCount, 20u);
}

// ========== 共享头状态机 ==========

TEST_CASE_FIXTURE(ShmTestFixture, "ShmTestFixture.MarkDisconnectedAndReportWhetherLastHolder_ReportsLastOwner")
{
    CHECK_FALSE(client_->MarkDisconnectedAndReportWhetherLastHolder());
    CHECK_EQ(header_->Status, ConnectStatusType::DisConnected);
    CHECK(client_->MarkDisconnectedAndReportWhetherLastHolder());
    CHECK_EQ(header_->Status, ConnectStatusType::DisConnected);
}

TEST_CASE_FIXTURE(ShmTestFixture, "ShmTestFixture.ResetSharedHeader_ClearsCountersAndStatus")
{
    client_->Write("Hello", 5);
    server_->Write("World", 5);
    CHECK_EQ(header_->UpWriteCount, 5u);
    CHECK_EQ(header_->DownWriteCount, 5u);

    client_->ResetSharedHeader();

    CHECK_EQ(header_->Status, ConnectStatusType::UnConnected);
    CHECK_EQ(header_->UpWriteCount, 0u);
    CHECK_EQ(header_->UpReadCount, 0u);
    CHECK_EQ(header_->DownWriteCount, 0u);
    CHECK_EQ(header_->DownReadCount, 0u);
}

// ========== 退让共享通道 ==========

TEST_CASE_FIXTURE(ShmTestFixture, "ShmTestFixture.MarkDisconnectedAndResetChannelWhenLastHolder_MarksDisconnectedWhenStillConnected")
{
    ShmBuffer<TestShmBufferSize> buffer(ServerTypeType::Client, 1, memory_.data(), ConnectStatusType::Connected, TestShmConnectCount);

    buffer.MarkDisconnectedAndResetChannelWhenLastHolder();

    CHECK_EQ(HeaderOfSecondConnection()->Status, ConnectStatusType::DisConnected);
}

TEST_CASE_FIXTURE(ShmTestFixture, "ShmTestFixture.MarkDisconnectedAndResetChannelWhenLastHolder_ResetsHeaderWhenAlreadyDisconnected")
{
    ShmBuffer<TestShmBufferSize> buffer(ServerTypeType::Client, 1, memory_.data(), ConnectStatusType::Connected, TestShmConnectCount);
    CHECK_EQ(buffer.Write("Hello", 5), 5u);
    CHECK_FALSE(buffer.MarkDisconnectedAndReportWhetherLastHolder());

    buffer.MarkDisconnectedAndResetChannelWhenLastHolder();

    CHECK_EQ(HeaderOfSecondConnection()->Status, ConnectStatusType::UnConnected);
    CHECK_EQ(HeaderOfSecondConnection()->UpWriteCount, 0u);
}

TEST_CASE_FIXTURE(ShmTestFixture, "ShmTestFixture.MarkDisconnectedAndResetChannelWhenLastHolder_LeavesTheNextEpochAnEmptyChannel")
{
    REQUIRE_EQ(client_->Write("Hello", 5), 5u);
    char upDestination[8] = {};
    REQUIRE_EQ(server_->Read(upDestination, 5), 5u);
    REQUIRE_EQ(server_->Write("World", 5), 5u);
    char downDestination[8] = {};
    REQUIRE_EQ(client_->Read(downDestination, 5), 5u);
    REQUIRE_EQ(header_->UpWriteCount, 5u);
    REQUIRE_EQ(header_->UpReadCount, 5u);
    REQUIRE_EQ(header_->DownWriteCount, 5u);
    REQUIRE_EQ(header_->DownReadCount, 5u);

    CHECK_FALSE(client_->MarkDisconnectedAndReportWhetherLastHolder());
    client_->MarkDisconnectedAndResetChannelWhenLastHolder();

    CHECK_EQ(header_->Status, ConnectStatusType::UnConnected);
    CHECK_EQ(header_->UpWriteCount, 0u);
    CHECK_EQ(header_->UpReadCount, 0u);
    CHECK_EQ(header_->DownWriteCount, 0u);
    CHECK_EQ(header_->DownReadCount, 0u);

    ShmBuffer<TestShmBufferSize> nextEpochClient(ServerTypeType::Client, 1, memory_.data(), ConnectStatusType::Connected, TestShmConnectCount);
    ShmBuffer<TestShmBufferSize> nextEpochServer(ServerTypeType::Server, 1, memory_.data(), ConnectStatusType::Connected, TestShmConnectCount);
    CHECK_EQ(nextEpochClient.GetReadBufferSize(), 0u);
    CHECK_EQ(nextEpochClient.GetWriteBufferSize(), TestShmBufferSize);
    CHECK_EQ(nextEpochServer.GetReadBufferSize(), 0u);
    CHECK_EQ(nextEpochServer.GetWriteBufferSize(), TestShmBufferSize);
}

// ========== 撤回未被对端确认的受理 ==========

TEST_CASE_FIXTURE(ShmTestFixture, "ShmTestFixture.RevokeUnconfirmedAccept_ReclaimsTheChannelWhenThePeerNeverConfirmed")
{
    client_.reset();
    server_.reset();
    ShmBuffer<TestShmBufferSize> unconfirmedPeer(ServerTypeType::Server, 1, memory_.data(), ConnectStatusType::Accepted, TestShmConnectCount);

    CHECK(unconfirmedPeer.RevokeUnconfirmedAccept());
    CHECK_EQ(HeaderOfSecondConnection()->Status, ConnectStatusType::DisConnected);
    CHECK_FALSE(unconfirmedPeer.RevokeUnconfirmedAccept());
    CHECK_EQ(HeaderOfSecondConnection()->Status, ConnectStatusType::DisConnected);
}

TEST_CASE_FIXTURE(ShmTestFixture, "ShmTestFixture.RevokeUnconfirmedAccept_LeavesAConfirmedChannelAlone")
{
    CHECK_FALSE(client_->RevokeUnconfirmedAccept());

    CHECK_EQ(header_->Status, ConnectStatusType::Connected);
}

TEST_CASE("ShmBufferTest.ConfirmAcceptedConnectionPublishesConnectedOnlyFromAccepted")
{
    SingleShmHeader channelHeader{};
    SingleShmHeader::StoreStatus(&channelHeader, ConnectStatusType::Accepted);

    CHECK(SingleShmHeader::ConfirmAcceptedConnection(&channelHeader));
    CHECK_EQ(SingleShmHeader::LoadStatus(&channelHeader), ConnectStatusType::Connected);

    CHECK_FALSE(SingleShmHeader::ConfirmAcceptedConnection(&channelHeader));
    CHECK_EQ(SingleShmHeader::LoadStatus(&channelHeader), ConnectStatusType::Connected);
}

TEST_CASE("ShmBufferTest.ConfirmAcceptedConnectionLeavesARevokedChannelUntouched")
{
    SingleShmHeader channelHeader{};
    SingleShmHeader::StoreStatus(&channelHeader, ConnectStatusType::DisConnected);

    CHECK_FALSE(SingleShmHeader::ConfirmAcceptedConnection(&channelHeader));
    CHECK_EQ(SingleShmHeader::LoadStatus(&channelHeader), ConnectStatusType::DisConnected);

    SingleShmHeader::StoreStatus(&channelHeader, ConnectStatusType::UnConnected);
    CHECK_FALSE(SingleShmHeader::ConfirmAcceptedConnection(&channelHeader));
    CHECK_EQ(SingleShmHeader::LoadStatus(&channelHeader), ConnectStatusType::UnConnected);
}

TEST_CASE("ShmBufferTest.RevokeConfirmedConnectionOnlyTakesAConfirmedChannel")
{
    SingleShmHeader channelHeader{};
    SingleShmHeader::StoreStatus(&channelHeader, ConnectStatusType::Connected);

    CHECK(SingleShmHeader::RevokeConfirmedConnection(&channelHeader));
    CHECK_EQ(SingleShmHeader::LoadStatus(&channelHeader), ConnectStatusType::DisConnected);

    CHECK_FALSE(SingleShmHeader::RevokeConfirmedConnection(&channelHeader));
    CHECK_EQ(SingleShmHeader::LoadStatus(&channelHeader), ConnectStatusType::DisConnected);

    SingleShmHeader::StoreStatus(&channelHeader, ConnectStatusType::UnConnected);
    CHECK_FALSE(SingleShmHeader::RevokeConfirmedConnection(&channelHeader));
    CHECK_EQ(SingleShmHeader::LoadStatus(&channelHeader), ConnectStatusType::UnConnected);

    SingleShmHeader::StoreStatus(&channelHeader, ConnectStatusType::Accepted);
    CHECK_FALSE(SingleShmHeader::RevokeConfirmedConnection(&channelHeader));
    CHECK_EQ(SingleShmHeader::LoadStatus(&channelHeader), ConnectStatusType::Accepted);
}
