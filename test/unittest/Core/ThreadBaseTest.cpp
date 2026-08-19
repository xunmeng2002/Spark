#include <gtest/gtest.h>
#include <Spark/Core/Thread/ThreadBase.h>

#include <thread>
#include <chrono>
#include <atomic>
using namespace spark::core;
// ============================================================
// ThreadBase 测试
// 使用 TestWorker 子类验证线程生命周期
// ============================================================

namespace
{
    class TestWorker : public ThreadBase
    {
    public:
        std::atomic<int> run_count_{0};
        std::atomic<bool> init_called_{false};
        std::atomic<bool> exit_called_{false};

        TestWorker(const char* name, int timeout = 100)
            : ThreadBase(name, timeout)
        {
        }

        bool IsJoinable() const
        {
            return m_Thread.joinable();
        }

    protected:
        void ThreadInit() override
        {
            init_called_ = true;
        }

        void ThreadExit() override
        {
            exit_called_ = true;
        }

        void Run() override
        {
            ++run_count_;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    };
}

// ---------- 构造 ----------

TEST(ThreadBaseTest, DefaultConstruction)
{
    TestWorker worker("TestThread", 200);

    EXPECT_FALSE(worker.IsJoinable());
    EXPECT_EQ(worker.GetThreadId(), std::thread::id());
}

// ---------- Start / Stop / Join ----------

TEST(ThreadBaseTest, StartAndStop)
{
    TestWorker worker("Worker1", 50);

    EXPECT_TRUE(worker.Start());
    EXPECT_TRUE(worker.IsJoinable());

    // 让线程运行一小段时间
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    worker.Stop();
    worker.Join();

    EXPECT_FALSE(worker.IsJoinable());
    EXPECT_GT(worker.run_count_, 0);
}

TEST(ThreadBaseTest, StartAndImmediateStop)
{
    TestWorker worker("Worker2", 100);

    EXPECT_TRUE(worker.Start());
    worker.Stop();
    worker.Join();

    // 即使立即停止，Run 也可能执行了若干次
    EXPECT_GE(worker.run_count_, 0);
    EXPECT_FALSE(worker.IsJoinable());
}

// ---------- 重复启动 ----------

TEST(ThreadBaseTest, DoubleStart_ReturnsFalse)
{
    TestWorker worker("Double", 100);

    EXPECT_TRUE(worker.Start());
    EXPECT_FALSE(worker.Start());  // 第二次启动返回 false
    EXPECT_TRUE(worker.IsJoinable());

    worker.Stop();
    worker.Join();
}

// ---------- 未启动直接 Join ----------

TEST(ThreadBaseTest, JoinWithoutStart)
{
    TestWorker worker("NoStart", 100);

    // 未启动的线程调用 Join 不崩溃
    EXPECT_NO_THROW(worker.Join());
}

// ---------- GetThreadId ----------

TEST(ThreadBaseTest, GetThreadId_AfterStart)
{
    TestWorker worker("WithID", 100);

    EXPECT_EQ(worker.GetThreadId(), std::thread::id());

    worker.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    EXPECT_NE(worker.GetThreadId(), std::thread::id());

    worker.Stop();
    worker.Join();
}

// ---------- ThreadInit / ThreadExit ----------

TEST(ThreadBaseTest, InitAndExitAreCalled)
{
    TestWorker worker("InitExit", 50);

    worker.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    worker.Stop();
    worker.Join();

    EXPECT_TRUE(worker.init_called_);
    EXPECT_TRUE(worker.exit_called_);
}

// ---------- 析构函数自动 Stop + Join ----------

TEST(ThreadBaseTest, DestructorStopsThread)
{
    {
        TestWorker worker("AutoStop", 50);
        worker.Start();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        // 离开作用域时自动 Stop + Join
    }
    // 能正常析构即通过
}

// ---------- SetTimeOut ----------

TEST(ThreadBaseTest, SetTimeOut_BeforeStart)
{
    TestWorker worker("Timeout", 100);

    EXPECT_NO_THROW(worker.SetTimeOut(200));
}

TEST(ThreadBaseTest, SetTimeOut_MultipleWorkers)
{
    TestWorker worker1("Multi1", 50);
    TestWorker worker2("Multi2", 50);

    EXPECT_TRUE(worker1.Start());
    EXPECT_TRUE(worker2.Start());

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    worker1.Stop();
    worker2.Stop();
    worker1.Join();
    worker2.Join();

    EXPECT_GT(worker1.run_count_, 0);
    EXPECT_GT(worker2.run_count_, 0);
}
