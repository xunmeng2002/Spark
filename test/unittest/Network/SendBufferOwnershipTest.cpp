#include <gtest/gtest.h>
#include <Spark/Network/Io/IoBase.h>
#include <Spark/Network/Io/IoFactory.h>
#include <Spark/TemplateLib/Buffer/LinearBuffer.h>

#include <memory>
using namespace Spark;
using namespace Spark::Network;

// ============================================================
// 跨模块发送缓冲的所有权
// ObjectPool 实例按模块各有一份，缓冲的归还在 IO 层内部完成，故应用侧必须向 IO 层
// 领取发送缓冲；自行 Allocate 得到的对象会被归还进非属主的池（Debug 下检测器中止）。
// ============================================================

TEST(SendBufferOwnershipTest, AllocatedSendBufferIsReturnedByIoLayer)
{
    const std::unique_ptr<IoBase> io(IoFactory::CreateIo(ServerTypeType::Server, "tcp://127.0.0.1:1"));
    ASSERT_NE(io, nullptr);

    LinearBuffer<BufferSize>* buffer = io->AllocateSendBuffer();
    ASSERT_NE(buffer, nullptr);
    EXPECT_GT(buffer->GetWriteBufferSize(), 0u);
    EXPECT_EQ(buffer->Append("Spark", 5), 5u);
    EXPECT_EQ(buffer->GetLength(), 5u);

    // 不存在的会话：Send 走「找不到连接即丢弃」路径，由 IO 层归还它自己发放的缓冲
    io->Send(0, buffer);
}
