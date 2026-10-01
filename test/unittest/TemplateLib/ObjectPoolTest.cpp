#include <gtest/gtest.h>
#include <Spark/TemplateLib/ObjectPool/ObjectPool.h>

#include <algorithm>
#include <atomic>
#include <mutex>
#include <set>
#include <stdexcept>
#include <thread>
#include <vector>
using namespace Spark;
// ============================================================
// ObjectPool 测试 — 无锁对象池（Allocate / Deallocate / AllocateShared）
//
// 注意：ObjectPool 是单例（按类型），每类 T 拥有独立实例。
// 测试使用自定义结构体作为类型参数，避免跨用例干扰。
// ============================================================

// ---------- 测试用数据类型 ----------

struct PoolInt
{
    PoolInt() : value(0) {}
    explicit PoolInt(int v) : value(static_cast<long long>(v)) {}

    long long value;
};

struct PoolPoint
{
    PoolPoint() : x(0), y(0) {}
    PoolPoint(int a, int b) : x(a), y(b) {}

    int x;
    int y;
};

// 检测构造/析构计数的类型
struct PoolTracked
{
    PoolTracked() : id(0) { s_Constructed++; }
    PoolTracked(int i) : id(static_cast<long long>(i)) { s_Constructed++; }
    ~PoolTracked() { s_Destroyed++; }

    static std::atomic<int> s_Constructed;
    static std::atomic<int> s_Destroyed;

    long long id;
};
std::atomic<int> PoolTracked::s_Constructed{0};
std::atomic<int> PoolTracked::s_Destroyed{0};

// ---------- Allocate ----------

TEST(ObjectPoolTest, Allocate_Default)
{
    PoolInt* obj = ObjectPool<PoolInt>::GetInstance().Allocate();
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->value, 0);
    ObjectPool<PoolInt>::GetInstance().Deallocate(obj);
}

TEST(ObjectPoolTest, Allocate_WithArgs)
{
    PoolPoint* obj = ObjectPool<PoolPoint>::GetInstance().Allocate(3, 7);
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->x, 3);
    EXPECT_EQ(obj->y, 7);
    ObjectPool<PoolPoint>::GetInstance().Deallocate(obj);
}

TEST(ObjectPoolTest, Allocate_ReturnsTheSlotToTheFreeListWhenConstructionThrows)
{
    struct ThrowingType
    {
        explicit ThrowingType(bool shouldRefuseConstruction)
        {
            if (shouldRefuseConstruction)
            {
                throw std::runtime_error("construction refused by test");
            }
        }

        long long value = 0;
    };

    ObjectPool<ThrowingType>& pool = ObjectPool<ThrowingType>::GetInstance();
    pool.SetBlockUnitNum(4);

    std::vector<ThrowingType*> slots;
    for (int i = 0; i < 4; ++i)
    {
        slots.push_back(pool.Allocate(false));
    }
    for (ThrowingType* slot : slots)
    {
        pool.Deallocate(slot);
    }
    std::sort(slots.begin(), slots.end());

    for (int i = 0; i < 3; ++i)
    {
        EXPECT_THROW(pool.Allocate(true), std::runtime_error);
    }

    std::vector<ThrowingType*> recycledSlots;
    for (int i = 0; i < 4; ++i)
    {
        ThrowingType* slot = pool.Allocate(false);
        ASSERT_NE(slot, nullptr);
        recycledSlots.push_back(slot);
    }
    std::sort(recycledSlots.begin(), recycledSlots.end());

    EXPECT_EQ(recycledSlots, slots);

    for (ThrowingType* slot : recycledSlots)
    {
        pool.Deallocate(slot);
    }
}

// ---------- Allocate / Deallocate 循环 ----------

TEST(ObjectPoolTest, AllocateAndDeallocate_Recycles)
{
    ObjectPool<PoolInt>& pool = ObjectPool<PoolInt>::GetInstance();

    PoolInt* a = pool.Allocate(42);
    ASSERT_NE(a, nullptr);
    pool.Deallocate(a);

    // Next allocation should reuse the same memory
    PoolInt* b = pool.Allocate(99);
    ASSERT_NE(b, nullptr);
    EXPECT_EQ(b->value, 99);
    pool.Deallocate(b);
}

TEST(ObjectPoolTest, MultipleAllocateDeallocate)
{
    ObjectPool<PoolInt>& pool = ObjectPool<PoolInt>::GetInstance();
    std::vector<PoolInt*> items;

    for (int i = 0; i < 10; ++i)
    {
        items.push_back(pool.Allocate(i * 10));
    }
    for (auto* item : items)
    {
        pool.Deallocate(item);
    }
    // Success if no crash / assertion failure
}

// ---------- AllocateShared ----------

TEST(ObjectPoolTest, AllocateShared_Basic)
{
    auto ptr = ObjectPool<PoolInt>::GetInstance().AllocateShared(42);
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(ptr->value, 42);

    // When shared_ptr goes out of scope, Deallocate is called automatically
}

TEST(ObjectPoolTest, AllocateShared_ReusesOnDeallocate)
{
    ObjectPool<PoolInt>& pool = ObjectPool<PoolInt>::GetInstance();

    {
        auto a = pool.AllocateShared(10);
        ASSERT_EQ(a->value, 10);
    }
    // a is destroyed, memory returned to pool

    auto b = pool.AllocateShared(20);
    ASSERT_EQ(b->value, 20);
    // Should have reused the same slot (verify by tracking count would be hard here,
    // but at minimum no crash and correct value)
}

// ---------- Deallocate(nullptr) 安全 ----------

TEST(ObjectPoolTest, Deallocate_Nullptr_Safe)
{
    ObjectPool<PoolInt>& pool = ObjectPool<PoolInt>::GetInstance();
    // Should not crash
    pool.Deallocate(nullptr);
}

// ---------- 扩容 ----------

TEST(ObjectPoolTest, Expand_Blocks)
{
    // 使用独立类型确保旧扩容不影响
    struct ExpandType
    {
        ExpandType() = default;
        char data[32];
    };

    ObjectPool<ExpandType>& pool = ObjectPool<ExpandType>::GetInstance();
    pool.SetBlockUnitNum(16); // 每块 16 个元素

    std::vector<ExpandType*> items;

    // 分配超过一块容量，触发 Expand
    for (int i = 0; i < 40; ++i)
    {
        items.push_back(pool.Allocate());
        ASSERT_NE(items.back(), nullptr);
    }

    // 全部归还
    for (auto* item : items)
    {
        pool.Deallocate(item);
    }
}

// ---------- 块容量取值门禁 ----------

TEST(ObjectPoolTest, SetBlockUnitNum_RejectsNonPositive)
{
    struct BlockUnitNumType
    {
        BlockUnitNumType() : value(0) {}

        long long value;
    };

    ObjectPool<BlockUnitNumType>& pool = ObjectPool<BlockUnitNumType>::GetInstance();
    EXPECT_THROW(pool.SetBlockUnitNum(0), std::invalid_argument);
    EXPECT_THROW(pool.SetBlockUnitNum(-8), std::invalid_argument);

    // 被拒绝后池仍可用：默认块容量未受影响，分配照常
    BlockUnitNumType* obj = pool.Allocate();
    ASSERT_NE(obj, nullptr);
    pool.Deallocate(obj);

    pool.SetBlockUnitNum(8); // 合法值照收
    BlockUnitNumType* item = pool.Allocate();
    ASSERT_NE(item, nullptr);
    pool.Deallocate(item);
}

// ---------- 线程本地缓存整批交回 ----------

TEST(ObjectPoolTest, RecycleAfterExcessReturnToSharedList)
{
    struct ExcessReturnType
    {
        ExcessReturnType() : value(0) {}

        long long value;
    };

    ObjectPool<ExcessReturnType>& pool = ObjectPool<ExcessReturnType>::GetInstance();
    pool.SetBlockUnitNum(8); // 小块 + 40 个对象：归还时必然多次触发本地缓存整批交回

    std::vector<ExcessReturnType*> allocated;
    for (int i = 0; i < 40; ++i)
    {
        ExcessReturnType* item = pool.Allocate();
        ASSERT_NE(item, nullptr);
        item->value = i;
        allocated.push_back(item);
    }
    for (ExcessReturnType* item : allocated)
    {
        pool.Deallocate(item);
    }

    // 全部归还后再同时取 40 个：必须全部来自原槽位（本地缓存有上界，节点已整批回到共享链），
    // 且互不重复——重复即同一节点被发出两次
    std::vector<ExcessReturnType*> recycled;
    for (int i = 0; i < 40; ++i)
    {
        ExcessReturnType* item = pool.Allocate();
        ASSERT_NE(item, nullptr);
        recycled.push_back(item);
    }

    std::vector<ExcessReturnType*> sortedRecycled = recycled;
    std::sort(sortedRecycled.begin(), sortedRecycled.end());
    EXPECT_EQ(std::adjacent_find(sortedRecycled.begin(), sortedRecycled.end()), sortedRecycled.end());

    for (int i = 0; i < 40; ++i)
    {
        EXPECT_NE(std::find(allocated.begin(), allocated.end(), recycled[i]), allocated.end());
        recycled[i]->value = 100 + i;
        EXPECT_EQ(recycled[i]->value, 100 + i);
    }

    for (ExcessReturnType* item : recycled)
    {
        pool.Deallocate(item);
    }
}

// ---------- 多线程分配 ----------

TEST(ObjectPoolTest, MultiThreadAllocate)
{
    struct MTPoolType
    {
        MTPoolType() : value(0) {}
        explicit MTPoolType(int v) : value(static_cast<long long>(v)) {}

        long long value;
    };

    ObjectPool<MTPoolType>& pool = ObjectPool<MTPoolType>::GetInstance();
    constexpr int PerThread = 100;
    constexpr int Threads = 4;

    std::vector<std::thread> threads;
    std::atomic<long long> sum{0};

    for (int t = 0; t < Threads; ++t)
    {
        threads.emplace_back(
            [&pool, &sum]()
            {
                for (int i = 0; i < PerThread; ++i)
                {
                    auto* obj = pool.Allocate(i);
                    sum.fetch_add(obj->value, std::memory_order_relaxed);
                    pool.Deallocate(obj);
                }
            });
    }

    for (auto& th : threads)
        th.join();

    // 每个线程分配 0..PerThread-1，共 Threads 个线程
    long long expected = static_cast<long long>(PerThread) * (PerThread - 1) / 2 * Threads;
    EXPECT_EQ(sum.load(), expected);

    // Clean up any remaining items left in pool
    // (no-op: pool destructor cleans all blocks)
}

// ---------- 跨线程归还 ----------

TEST(ObjectPoolTest, CrossThreadDeallocate)
{
    struct CrossThreadType
    {
        CrossThreadType() : value(0) {}

        long long value;
    };

    constexpr int ItemCount = 64;
    ObjectPool<CrossThreadType>& pool = ObjectPool<CrossThreadType>::GetInstance();

    std::vector<CrossThreadType*> allocated;
    for (int i = 0; i < ItemCount; ++i)
    {
        CrossThreadType* item = pool.Allocate();
        ASSERT_NE(item, nullptr);
        item->value = i;
        allocated.push_back(item);
    }

    // 换一个线程归还：节点先落到归还线程自己的本地链上，之后必须经共享链重新可取
    std::thread releaser(
        [&pool, &allocated]()
        {
            for (CrossThreadType* item : allocated)
            {
                pool.Deallocate(item);
            }
        });
    releaser.join();

    for (int i = 0; i < ItemCount; ++i)
    {
        CrossThreadType* item = pool.Allocate();
        ASSERT_NE(item, nullptr);
        item->value = 900 + i;
        EXPECT_EQ(item->value, 900 + i);
        pool.Deallocate(item);
    }
}

// ---------- 多线程并发取还与整批搬运 ----------

TEST(ObjectPoolTest, ConcurrentBatchAllocateDeallocate)
{
    struct ConcurrentBatchType
    {
        ConcurrentBatchType() : value(0) {}

        long long value;
    };

    constexpr int ThreadCount = 8;
    constexpr int RoundCount = 40;
    constexpr int BatchSize = 12; // 每线程每轮同时持有 12 个（远超本地上限的一半），反复触发整批取回与整批交回

    ObjectPool<ConcurrentBatchType>& pool = ObjectPool<ConcurrentBatchType>::GetInstance();
    pool.SetBlockUnitNum(16);

    // 「同一地址不得同时属于两个线程」是本池的核心不变量，ABA 破坏的正是它。
    // 每轮把各线程持有的地址登记进共享集合，重复登记即同一节点被发出两次。
    std::mutex livePointerMutex;
    std::set<ConcurrentBatchType*> livePointers;
    std::atomic<bool> duplicatePointerSeen{false};

    std::vector<std::thread> workers;
    workers.reserve(ThreadCount);
    for (int t = 0; t < ThreadCount; ++t)
    {
        workers.emplace_back(
            [&pool, &livePointerMutex, &livePointers, &duplicatePointerSeen, t]()
            {
                for (int round = 0; round < RoundCount; ++round)
                {
                    std::vector<ConcurrentBatchType*> batch;
                    batch.reserve(BatchSize);
                    for (int i = 0; i < BatchSize; ++i)
                    {
                        ConcurrentBatchType* item = pool.Allocate();
                        item->value = static_cast<long long>(t) * 1000 + i;
                        batch.push_back(item);

                        std::lock_guard<std::mutex> guard(livePointerMutex);
                        if (!livePointers.insert(item).second)
                        {
                            duplicatePointerSeen.store(true);
                        }
                    }

                    for (ConcurrentBatchType* item : batch)
                    {
                        {
                            std::lock_guard<std::mutex> guard(livePointerMutex);
                            livePointers.erase(item);
                        }
                        pool.Deallocate(item);
                    }
                }
            });
    }

    for (std::thread& worker : workers)
    {
        worker.join();
    }

    ASSERT_FALSE(duplicatePointerSeen.load());
    EXPECT_TRUE(livePointers.empty());

    // 全部归还后池仍可正常取还
    ConcurrentBatchType* item = pool.Allocate();
    ASSERT_NE(item, nullptr);
    item->value = 7;
    EXPECT_EQ(item->value, 7);
    pool.Deallocate(item);
}

// ---------- 线程退出回收 ----------

TEST(ObjectPoolTest, ThreadExit_ReturnsThreadLocalNodesToSharedList)
{
    struct ThreadExitReclaimType
    {
        long long value = 0;
    };

    ObjectPool<ThreadExitReclaimType>& pool = ObjectPool<ThreadExitReclaimType>::GetInstance();
    pool.SetBlockUnitNum(4);

    std::set<ThreadExitReclaimType*> exitedThreadNodes;
    std::thread allocatingThread(
        [&pool, &exitedThreadNodes]()
        {
            std::vector<ThreadExitReclaimType*> items;
            for (int i = 0; i < 4; ++i)
            {
                items.push_back(pool.Allocate());
            }
            for (ThreadExitReclaimType* item : items)
            {
                exitedThreadNodes.insert(item);
                pool.Deallocate(item);
            }
        });
    allocatingThread.join();

    std::vector<ThreadExitReclaimType*> reclaimedSlots;
    for (int i = 0; i < 4; ++i)
    {
        reclaimedSlots.push_back(pool.Allocate());
    }

    const std::set<ThreadExitReclaimType*> reclaimedNodes(reclaimedSlots.begin(), reclaimedSlots.end());
    EXPECT_EQ(reclaimedNodes, exitedThreadNodes);

    for (ThreadExitReclaimType* slot : reclaimedSlots)
    {
        pool.Deallocate(slot);
    }
}

// ---------- 析构/构造计数 ----------

TEST(ObjectPoolTest, TrackedType_ConstructAndDestroy)
{
    // 重置计数（注意类型是 PoolTracked，各测试共享此静态计数）
    // 由于单例特性，前面可能有残留计数，此处只验证分配/归还增减一致
    auto beforeConstruct = PoolTracked::s_Constructed.load();
    auto beforeDestroy = PoolTracked::s_Destroyed.load();

    PoolTracked* obj = ObjectPool<PoolTracked>::GetInstance().Allocate(5);
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->id, 5);

    ObjectPool<PoolTracked>::GetInstance().Deallocate(obj);

    // Deallocate 调用了析构函数，但不释放内存（还给 pool）
    // 所以 Destroyed 增加，Constructed 不变（归还后 Allocate 通过 placement-new 重用）
    EXPECT_GT(PoolTracked::s_Destroyed.load(), beforeDestroy);
}
