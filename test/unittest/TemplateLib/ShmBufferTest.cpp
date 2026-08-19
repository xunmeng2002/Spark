#include <gtest/gtest.h>
#include <Spark/TemplateLib/Buffer/ShmBuffer.h>

#include <cstring>
#include <string>
#include <vector>
using namespace spark;
// ============================================================
// ShmBuffer 测试 — 共享内存缓冲区
// 使用 malloc 模拟共享内存，验证数据写入/读取路径
// ============================================================

static constexpr unsigned kShmSize = 256;

// 为 ShmBuffer 分配足够大的模拟内存并初始化 header
// 使用 index=1 避免 index=0 时 header 与 up_buffer 重叠
struct ShmTestFixture : public ::testing::Test
{
    void SetUp() override
    {
        // 内存布局 (index=1):
        //   [0 .. sizeof(SingleShmHeader))  — header (index=0)
        //   [sizeof(SingleShmHeader) .. 2*SIZE) — 填充
        //   [2*SIZE .. 3*SIZE)              — UpBuffer
        //   [3*SIZE .. 4*SIZE)              — DownBuffer
        size_t total = sizeof(SingleShmHeader) + kShmSize * 4;
        memory_.resize(total, 0);

        // 使用 index=1 构造 — ShmBuffer 自行定位 header、UpBuffer、DownBuffer
        client_.reset(new ShmBuffer<kShmSize>(
            ServerTypeType::Client, 1, memory_.data(), ConnectStatusType::Connected));
        server_.reset(new ShmBuffer<kShmSize>(
            ServerTypeType::Server, 1, memory_.data(), ConnectStatusType::Connected));

        // header_ 指向 ShmBuffer 实际使用的 header（index=1，偏移 sizeof(SingleShmHeader)）
        header_ = client_->m_ShmHeader;
    }

    void TearDown() override
    {
        client_.reset();
        server_.reset();
    }

    std::vector<char> memory_;
    SingleShmHeader* header_;
    std::unique_ptr<ShmBuffer<kShmSize>> client_;
    std::unique_ptr<ShmBuffer<kShmSize>> server_;
};

// ========== 构造 ==========

TEST(ShmBufferTest, StatusConnected)
{
    std::vector<char> mem(sizeof(SingleShmHeader) + kShmSize * 4, 0);
    SingleShmHeader* h = reinterpret_cast<SingleShmHeader*>(mem.data());
    h->Status = ConnectStatusType::Connected;

    ShmBuffer<kShmSize> buf(ServerTypeType::Server, 1, mem.data(), ConnectStatusType::Connected);
    ConnectStatusType status = buf.m_ShmHeader->Status;
    EXPECT_EQ(status, ConnectStatusType::Connected);
}

// ========== Client 写（UpWrite）/ 读（DownRead）==========

TEST_F(ShmTestFixture, ClientWrite_UpBuffer)
{
    const char* data = "HelloShm";
    unsigned written = client_->Write(data, 8);
    EXPECT_EQ(written, 8u);

    // Client Write → UpWrite → data in UpBuffer
    EXPECT_EQ(std::memcmp(memory_.data() + kShmSize * 2, data, 8), 0);
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
    unsigned read = client_->Read(output, 10);
    EXPECT_EQ(read, 10u);
    EXPECT_EQ(std::memcmp(output, "FromServer", 10), 0);
    EXPECT_EQ(header_->DownReadCount, 10u);
}

// ========== Server 写（DownWrite）/ 读（UpRead）==========

TEST_F(ShmTestFixture, ServerWrite_DownBuffer)
{
    const char* data = "ServerData";
    unsigned written = server_->Write(data, 10);
    EXPECT_EQ(written, 10u);

    // Server Write → DownWrite → data in DownBuffer
    EXPECT_EQ(std::memcmp(memory_.data() + kShmSize * 3, data, 10), 0);
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
    unsigned read = server_->Read(output, 10);
    EXPECT_EQ(read, 10u);
    EXPECT_EQ(std::memcmp(output, "FromClient", 10), 0);
    EXPECT_EQ(header_->UpReadCount, 10u);
}

// ========== 断连状态 ==========

TEST_F(ShmTestFixture, WriteWhenDisconnected_ReturnsZero)
{
    header_->Status = ConnectStatusType::DisConnected;
    EXPECT_EQ(client_->Write("data", 4), 0u);
    EXPECT_EQ(server_->Write("data", 4), 0u);
}

TEST_F(ShmTestFixture, ReadWhenDisconnected_ReturnsZero)
{
    header_->Status = ConnectStatusType::DisConnected;
    char buf[16] = {};
    EXPECT_EQ(client_->Read(buf, 4), 0u);
    EXPECT_EQ(server_->Read(buf, 4), 0u);
}

// ========== 缓冲区大小查询 ==========

TEST_F(ShmTestFixture, GetWriteBufferSize_Client)
{
    // Client WriteBufferSize = GetUpWriteBufferSize
    // 初始: SIZE - 0 - 1 = kShmSize - 1
    EXPECT_EQ(client_->GetWriteBufferSize(), kShmSize - 1);

    client_->Write("Hello", 5);
    // UpWriteCount = 5, UpReadCount = 0
    // GetUpWriteBufferSize: Read > Write? no, so SIZE - (5 - 0) - 1 = SIZE - 6
    EXPECT_EQ(client_->GetWriteBufferSize(), kShmSize - 6);
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
    EXPECT_EQ(server_->GetWriteBufferSize(), kShmSize - 1);

    server_->Write("Hello", 5);
    EXPECT_EQ(server_->GetWriteBufferSize(), kShmSize - 6);
}

TEST_F(ShmTestFixture, GetReadBufferSize_Server)
{
    EXPECT_EQ(server_->GetReadBufferSize(), 0u);

    client_->Write("Data", 4);
    EXPECT_EQ(server_->GetReadBufferSize(), 4u);
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
    unsigned written = client_->Write(second.data(), 105);
    EXPECT_EQ(written, 105u);
    EXPECT_EQ(header_->UpWriteCount, 99u); // 105 - 6 = 99 (绕回值)

    // Step 4: Server 读取全部数据
    //   内存布局: [0..99)=B, [100..250)=A, [250..256)=B
    char output[kShmSize] = {};
    unsigned read = server_->Read(output, kShmSize);
    EXPECT_EQ(read, 255u);

    // 前 150 字节是为读取的 'A' (从位置 100 到 249)
    for (unsigned i = 0; i < 150; ++i)
        EXPECT_EQ(output[i], 'A');
    // 后 105 字节为 'B' (尾部 6 字节 + 头部 99 字节)
    for (unsigned i = 150; i < 255; ++i)
        EXPECT_EQ(output[i], 'B');
}

// ========== 多次 Write / Read 周期 ==========

TEST_F(ShmTestFixture, MultiCycle)
{
    for (int i = 0; i < 5; ++i)
    {
        std::string msg = "Msg" + std::to_string(i);
        client_->Write(msg.data(), static_cast<unsigned>(msg.size()));
    }
    EXPECT_EQ(header_->UpWriteCount, 5 * 4); // "Msg0".."Msg4" each 4 bytes

    for (int i = 0; i < 5; ++i)
    {
        char output[16] = {};
        server_->Read(output, 4);
        std::string expected = "Msg" + std::to_string(i);
        EXPECT_EQ(std::memcmp(output, expected.data(), 4), 0);
    }
    EXPECT_EQ(header_->UpReadCount, 5 * 4);
}
