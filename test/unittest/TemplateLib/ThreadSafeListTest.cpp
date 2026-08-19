#include <gtest/gtest.h>
#include <Spark/TemplateLib/ThreadSafeContainer/ThreadSafeList.h>

#include <algorithm>
#include <numeric>
#include <thread>
#include <vector>
using namespace spark;
// ============================================================
// ThreadSafeList 测试 — 线程安全阻塞队列（PushBack / PopFront）
//
// PopFront 在队列为空时阻塞等待，因此测试需保证先有数据再消费。
// ============================================================

// ---------- 基本 Push/Pop ----------

TEST(ThreadSafeListTest, PushBackAndPopFront_Single)
{
    ThreadSafeList<int> list;
    int item = 42;
    list.PushBack(&item);

    int* result = list.PopFront();
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(*result, 42);
}

TEST(ThreadSafeListTest, PushBackAndPopFront_Multiple_FIFO)
{
    ThreadSafeList<int> list;
    std::vector<int> items(10);
    std::iota(items.begin(), items.end(), 0);

    for (auto& v : items)
    {
        list.PushBack(&v);
    }

    for (int i = 0; i < 10; ++i)
    {
        int* result = list.PopFront();
        ASSERT_NE(result, nullptr);
        EXPECT_EQ(*result, i);
    }
}

TEST(ThreadSafeListTest, InterleavedPushPop)
{
    ThreadSafeList<int> list;
    int a = 1, b = 2, c = 3;

    list.PushBack(&a);
    list.PushBack(&b);
    EXPECT_EQ(*list.PopFront(), 1);

    list.PushBack(&c);
    EXPECT_EQ(*list.PopFront(), 2);
    EXPECT_EQ(*list.PopFront(), 3);
}

// ---------- 拷贝构造 ----------

TEST(ThreadSafeListTest, CopyConstructor)
{
    ThreadSafeList<int> original;
    std::vector<int> items = {1, 2, 3};
    for (auto& v : items)
    {
        original.PushBack(&v);
    }

    ThreadSafeList<int> copy(original);
    // 原列表仍有数据
    EXPECT_EQ(*original.PopFront(), 1);
    // 拷贝也有数据
    EXPECT_EQ(*copy.PopFront(), 1);
    EXPECT_EQ(*copy.PopFront(), 2);
    EXPECT_EQ(*copy.PopFront(), 3);
}

// ---------- 多线程：单生产 + 单消费 ----------

TEST(ThreadSafeListTest, SingleProducerSingleConsumer)
{
    ThreadSafeList<int> list;
    constexpr int kItemCount = 1000;
    std::vector<int> items(kItemCount);
    std::iota(items.begin(), items.end(), 0);

    std::vector<int> results;

    std::thread producer([&]()
        {
            for (int i = 0; i < kItemCount; ++i)
            {
                list.PushBack(&items[i]);
            }
        });

    std::thread consumer([&]()
        {
            for (int i = 0; i < kItemCount; ++i)
            {
                int* item = list.PopFront();
                results.push_back(*item);
            }
        });

    producer.join();
    consumer.join();

    ASSERT_EQ(results.size(), static_cast<size_t>(kItemCount));
    for (int i = 0; i < kItemCount; ++i)
    {
        EXPECT_EQ(results[i], i);
    }
}

// ---------- 多线程：多生产 + 单消费 ----------

TEST(ThreadSafeListTest, MultiProducerSingleConsumer)
{
    ThreadSafeList<int> list;
    constexpr int kProducers = 4;
    constexpr int kItemsPerProducer = 500;
    constexpr int kTotalItems = kProducers * kItemsPerProducer;

    // 预分配存储，确保指针生命周期
    std::vector<int> itemStorage(kTotalItems);
    std::iota(itemStorage.begin(), itemStorage.end(), 0);

    std::vector<std::thread> producers;
    for (int t = 0; t < kProducers; ++t)
    {
        producers.emplace_back([&, t]()
            {
                int start = t * kItemsPerProducer;
                for (int i = 0; i < kItemsPerProducer; ++i)
                {
                    list.PushBack(&itemStorage[start + i]);
                }
            });
    }

    std::vector<int> results;
    std::thread consumer([&]()
        {
            for (int i = 0; i < kTotalItems; ++i)
            {
                int* item = list.PopFront();
                results.push_back(*item);
            }
            // 按值排序验证所有元素都收到，不做顺序保证
            std::sort(results.begin(), results.end());
        });

    for (auto& p : producers)
        p.join();
    consumer.join();

    // 验证收到全部元素
    ASSERT_EQ(results.size(), static_cast<size_t>(kTotalItems));
    for (int i = 0; i < kTotalItems; ++i)
    {
        EXPECT_EQ(results[i], i);
    }
}
