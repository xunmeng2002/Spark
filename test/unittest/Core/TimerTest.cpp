#include <gtest/gtest.h>
#include <Spark/Core/Timer/Timer.h>

#include <thread>
#include <chrono>
using namespace spark::core;
// ============================================================
// Timer 测试
// 使用 TestTimer 子类验证定时器触发逻辑
// ============================================================

namespace
{
    class TestTimer : public Timer
    {
    public:
        int on_timer_call_count_ = 0;

        using Timer::CheckTimer;

        int GetTimeInterval() const
        {
            return m_TimeInterval;
        }

        int GetEventCount() const
        {
            return m_EventCount;
        }

        int GetCurrentEventCount() const
        {
            return m_CurrentEventCount;
        }

    protected:
        void OnTimer() override
        {
            ++on_timer_call_count_;
        }
    };
}

// ---------- 默认构造 ----------

TEST(TimerTest, DefaultValues)
{
    TestTimer timer;

    // 默认：60 秒间隔，每 100ms 轮询一次 → 600 次轮询后检查时间
    EXPECT_EQ(timer.GetTimeInterval(), 60000);
    EXPECT_EQ(timer.GetEventCount(), 600);
    EXPECT_EQ(timer.GetCurrentEventCount(), 0);
}

// ---------- SetTimer ----------

TEST(TimerTest, SetTimer_CalculatesEventCount)
{
    TestTimer timer;

    // interval=1000ms, poll=100ms → eventCount=10
    timer.SetTimer(1000, 100);
    EXPECT_EQ(timer.GetTimeInterval(), 1000);
    EXPECT_EQ(timer.GetEventCount(), 10);
}

TEST(TimerTest, SetTimer_LargeInterval)
{
    TestTimer timer;

    timer.SetTimer(5000, 500);
    EXPECT_EQ(timer.GetTimeInterval(), 5000);
    EXPECT_EQ(timer.GetEventCount(), 10);
}

TEST(TimerTest, SetTimer_OneToOne)
{
    TestTimer timer;

    // 轮询间隔 = 触发间隔 → eventCount=1（每次 CheckTimer 都检查时间）
    timer.SetTimer(100, 100);
    EXPECT_EQ(timer.GetTimeInterval(), 100);
    EXPECT_EQ(timer.GetEventCount(), 1);
}

// ---------- CheckTimer: 未触发 ----------

TEST(TimerTest, CheckTimer_NotFiredBeforeEventCount)
{
    TestTimer timer;
    timer.SetTimer(1000, 100);  // eventCount=10

    // 轮询次数未超过 eventCount 时不应触发
    for (int i = 0; i < 10; ++i)
    {
        timer.CheckTimer();
    }
    EXPECT_EQ(timer.on_timer_call_count_, 0);
}

TEST(TimerTest, CheckTimer_FiresAfterElapsedTime)
{
    TestTimer timer;
    timer.SetTimer(50, 10);  // interval=50ms, eventCount=5

    // 轮询次数超过 eventCount（>5）+ 等待时间超过 interval（50ms）
    // 先等足够时间
    std::this_thread::sleep_for(std::chrono::milliseconds(60));

    // 再调用 CheckTimer 超过 eventCount 次
    for (int i = 0; i < 10; ++i)
    {
        timer.CheckTimer();
    }

    EXPECT_EQ(timer.on_timer_call_count_, 1);
}

// ---------- CheckTimer: 多次触发 ----------

TEST(TimerTest, CheckTimer_MultipleFires)
{
    TestTimer timer;
    timer.SetTimer(30, 5);  // interval=30ms, eventCount=6

    // 第一次触发
    std::this_thread::sleep_for(std::chrono::milliseconds(40));
    for (int i = 0; i < 10; ++i)
    {
        timer.CheckTimer();
    }
    EXPECT_EQ(timer.on_timer_call_count_, 1);

    // 第二次触发
    std::this_thread::sleep_for(std::chrono::milliseconds(40));
    for (int i = 0; i < 10; ++i)
    {
        timer.CheckTimer();
    }
    EXPECT_EQ(timer.on_timer_call_count_, 2);
}

// ---------- CheckTimer: 时间未到不触发 ----------

TEST(TimerTest, CheckTimer_DoesNotFireIfTimeNotElapsed)
{
    TestTimer timer;
    timer.SetTimer(10000, 50);  // interval=10s, eventCount=200

    // 轮询次数超过 eventCount，但时间没到
    for (int i = 0; i < 250; ++i)
    {
        timer.CheckTimer();
    }

    // 时间远未到 10s，不应触发
    EXPECT_EQ(timer.on_timer_call_count_, 0);
}

// ---------- CheckTimer: 触发后重置计数值 ----------

TEST(TimerTest, CheckTimer_ResetsCounterAfterFire)
{
    TestTimer timer;
    timer.SetTimer(30, 10);  // interval=30ms, eventCount=3

    std::this_thread::sleep_for(std::chrono::milliseconds(40));
    for (int i = 0; i < 10; ++i)
    {
        timer.CheckTimer();
    }

    // 触发后 m_CurrentEventCount 重置为 0（见 Timer.cpp CheckTimer 内 m_CurrentEventCount = 0）
    // 但后续 CheckTimer 会继续递增。10 次调用流程：
    // 第 1-3 次：count=1,2,3（3>3 不成立，不触发）
    // 第 4 次：count=4（4>3 成立，时间已到 → 触发 OnTimer，重置为 0）
    // 第 5-10 次：count=1,2,3,4,5,6（时间未再次流逝，不触发）
    EXPECT_EQ(timer.on_timer_call_count_, 1);
    EXPECT_EQ(timer.GetCurrentEventCount(), 6);
}
