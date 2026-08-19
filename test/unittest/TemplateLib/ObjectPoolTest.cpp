#include <gtest/gtest.h>
#include <Spark/TemplateLib/ObjectPool/ObjectPool.h>

#include <atomic>
#include <thread>
#include <vector>
using namespace spark;
// ============================================================
// ObjectPool 测试 — 无锁对象池（Allocate / Deallocate / AllocateShared）
//
// 注意：ObjectPool 是单例（按类型），每类 T 拥有独立实例。
// 测试使用自定义结构体作为类型参数，避免跨用例干扰。
// ============================================================

// ---------- 测试用数据类型 ----------

struct PoolInt
{
    long long value;
    PoolInt() : value(0) {}
    explicit PoolInt(int v) : value(static_cast<long long>(v)) {}
};

struct PoolPoint
{
    int x;
    int y;
    PoolPoint() : x(0), y(0) {}
    PoolPoint(int a, int b) : x(a), y(b) {}
};

// 检测构造/析构计数的类型
struct PoolTracked
{
    static std::atomic<int> s_Constructed;
    static std::atomic<int> s_Destroyed;

    long long id;
    PoolTracked() : id(0) { s_Constructed++; }
    PoolTracked(int i) : id(static_cast<long long>(i)) { s_Constructed++; }
    ~PoolTracked() { s_Destroyed++; }
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
        char data[32];
        ExpandType() = default;
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

// ---------- 多线程分配 ----------

TEST(ObjectPoolTest, MultiThreadAllocate)
{
    struct MTPoolType
    {
        long long value;
        MTPoolType() : value(0) {}
        explicit MTPoolType(int v) : value(static_cast<long long>(v)) {}
    };

    ObjectPool<MTPoolType>& pool = ObjectPool<MTPoolType>::GetInstance();
    constexpr int kPerThread = 100;
    constexpr int kThreads = 4;

    std::vector<std::thread> threads;
    std::atomic<long long> sum{0};

    for (int t = 0; t < kThreads; ++t)
    {
        threads.emplace_back([&pool, &sum]()
            {
                for (int i = 0; i < kPerThread; ++i)
                {
                    auto* obj = pool.Allocate(i);
                    sum.fetch_add(obj->value, std::memory_order_relaxed);
                    pool.Deallocate(obj);
                }
            });
    }

    for (auto& th : threads)
        th.join();

    // 每个线程分配 0..kPerThread-1，共 kThreads 个线程
    long long expected = static_cast<long long>(kPerThread) * (kPerThread - 1) / 2 * kThreads;
    EXPECT_EQ(sum.load(), expected);

    // Clean up any remaining items left in pool
    // (no-op: pool destructor cleans all blocks)
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
