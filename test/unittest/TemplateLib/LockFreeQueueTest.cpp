#include <Spark/TemplateLib/ThreadSafeContainer/LockFreeQueue.h>
#include "doctest/doctest.h"

#include <memory>
#include <string>
#include <thread>
#include <vector>
using namespace Spark;
// ============================================================
// LockFreeQueue 测试 — 无锁队列（PushBack / PopFront）
// ============================================================

// ---------- 初始状态 ----------

TEST_CASE("LockFreeQueueTest.DefaultConstructor_Empty")
{
    LockFreeQueue<int> q;
    CHECK(q.Empty());
    CHECK_EQ(q.PopFront(), nullptr);
}

// ---------- PushBack / PopFront ----------

TEST_CASE("LockFreeQueueTest.PushBack_Single")
{
    LockFreeQueue<int> q;
    auto data = std::make_shared<int>(42);
    CHECK(q.Empty());

    q.PushBack(data);
    CHECK_FALSE(q.Empty());

    auto result = q.PopFront();
    REQUIRE_NE(result, nullptr);
    CHECK_EQ(*result, 42);
    CHECK(q.Empty());
}

TEST_CASE("LockFreeQueueTest.PopFront_Empty_ReturnsNull")
{
    LockFreeQueue<int> q;
    CHECK_EQ(q.PopFront(), nullptr);
}

TEST_CASE("LockFreeQueueTest.PushBack_Multiple_FIFO")
{
    LockFreeQueue<int> q;

    for (int i = 0; i < 10; ++i)
    {
        q.PushBack(std::make_shared<int>(i));
    }

    for (int i = 0; i < 10; ++i)
    {
        auto result = q.PopFront();
        REQUIRE_NE(result, nullptr);
        CHECK_EQ(*result, i);
    }

    CHECK(q.Empty());
}

TEST_CASE("LockFreeQueueTest.InterleavedPushPop")
{
    LockFreeQueue<int> q;

    q.PushBack(std::make_shared<int>(1));
    q.PushBack(std::make_shared<int>(2));

    CHECK_EQ(*q.PopFront(), 1);

    q.PushBack(std::make_shared<int>(3));

    CHECK_EQ(*q.PopFront(), 2);
    CHECK_EQ(*q.PopFront(), 3);
    CHECK(q.Empty());
}

// ---------- shared_ptr 语义 ----------

TEST_CASE("LockFreeQueueTest.PushBack_SharedPtrAliasing")
{
    // 验证 queue 接管了 shared_ptr，内部不会提前释放
    LockFreeQueue<int> q;
    auto data = std::make_shared<int>(99);
    std::weak_ptr<int> weak = data;

    q.PushBack(data);
    data.reset(); // 释放外部引用

    // queue 内部仍持有共享所有权
    CHECK_FALSE(weak.expired());

    auto result = q.PopFront();
    REQUIRE_NE(result, nullptr);
    CHECK_EQ(*result, 99);

    // PopFront 后 queue 释放，weak 到期
    result.reset();
    CHECK(weak.expired());
}

// ---------- 字符串类型 ----------

TEST_CASE("LockFreeQueueTest.StringType")
{
    LockFreeQueue<std::string> q;
    q.PushBack(std::make_shared<std::string>("Hello"));
    q.PushBack(std::make_shared<std::string>("World"));

    auto a = q.PopFront();
    auto b = q.PopFront();
    REQUIRE_NE(a, nullptr);
    REQUIRE_NE(b, nullptr);
    CHECK_EQ(*a, "Hello");
    CHECK_EQ(*b, "World");
}

// ---------- 多线程 ----------

TEST_CASE("LockFreeQueueTest.MultiThread_SingleProducerSingleConsumer")
{
    LockFreeQueue<int> q;
    constexpr int ItemCount = 10000;

    // Producer
    std::thread producer(
        [&]()
        {
            for (int i = 0; i < ItemCount; ++i)
            {
                q.PushBack(std::make_shared<int>(i));
            }
        });

    // Consumer — 非阻塞 PopFront，需要不断轮询直到取完
    std::vector<int> results;
    std::thread consumer(
        [&]()
        {
            int received = 0;
            while (received < ItemCount)
            {
                auto item = q.PopFront();
                if (item)
                {
                    results.push_back(*item);
                    ++received;
                }
                else
                {
                    // 队列空时短暂让出 CPU
                    std::this_thread::yield();
                }
            }
        });

    producer.join();
    consumer.join();

    // 验证收到全部数据并按 FIFO 顺序
    REQUIRE_EQ(results.size(), static_cast<size_t>(ItemCount));
    for (int i = 0; i < ItemCount; ++i)
    {
        CHECK_EQ(results[i], i);
    }
}

// ---------- 析构安全 ----------

TEST_CASE("LockFreeQueueTest.DestructorWithRemainingItems")
{
    // 验证析构函数能安全清理非空队列
    auto q = std::make_unique<LockFreeQueue<int>>();
    for (int i = 0; i < 10; ++i)
    {
        q->PushBack(std::make_shared<int>(i));
    }
    // 不 Pop 直接析构 — 不应 crash
    q.reset();
}
