#pragma once
#include <cstddef>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <utility>
#ifndef NDEBUG
#include <cassert>
#include <cstdio>
#include <typeinfo>
#include <unordered_set>
#endif

namespace Spark
{
namespace ObjectPoolDetail
{
// 槽位步长：槽位必须同时容下 T 与压在 T 首部的空闲节点，两者对齐要求不同，
// 故把 sizeof(T) 按节点所需的对齐向上取整，尾部补齐
template <typename T, typename FreeNode>
constexpr size_t SlotByteCountFor = ((sizeof(T) + alignof(FreeNode) - 1) / alignof(FreeNode)) * alignof(FreeNode);
}

template <typename T>
class ObjectPool
{
public:
    static ObjectPool& GetInstance()
    {
        static ObjectPool instance_;
        return instance_;
    }

    void SetBlockUnitNum(int blockUnitNum)
    {
        if (blockUnitNum <= 0)
        {
            throw std::invalid_argument("ObjectPool::SetBlockUnitNum requires a positive blockUnitNum");
        }
        blockUnitNum_ = blockUnitNum;
    }

    template <typename... Args>
    T* Allocate(Args&&... args)
    {
        if (threadLocalCache_.FreeListHead == nullptr)
        {
            RefillThreadLocalFreeList();
        }

        FreeNode* node = threadLocalCache_.FreeListHead;
        threadLocalCache_.FreeListHead = node->Next;
        --threadLocalCache_.FreeNodeCount;

        T* obj = reinterpret_cast<T*>(node);
        new (obj) T(std::forward<Args>(args)...);
#ifndef NDEBUG
        RegisterAllocatedItem(obj);
#endif
        return obj;
    }

    template <typename... Args>
    std::shared_ptr<T> AllocateShared(Args&&... args)
    {
        T* obj = Allocate(std::forward<Args>(args)...);
        return std::shared_ptr<T>(obj, [](T* ptr) { ObjectPool<T>::GetInstance().Deallocate(ptr); });
    }

    void Deallocate(T* item)
    {
        if (item == nullptr) [[unlikely]]
            return;

#ifndef NDEBUG
        AssertAndUnregisterOwnedItem(item);
#endif
        item->~T();
        FreeNode* node = reinterpret_cast<FreeNode*>(item);
        node->Next = threadLocalCache_.FreeListHead;
        threadLocalCache_.FreeListHead = node;
        ++threadLocalCache_.FreeNodeCount;

        if (threadLocalCache_.FreeNodeCount > blockUnitNum_)
        {
            ReturnExcessThreadLocalNodesToSharedList();
        }
    }

private:
    struct Block
    {
        Block(T* objs, Block* next) : Objects(objs), Next(next) {}

        T* Objects;
        Block* Next;
    };
    // 空闲节点与 T 复用同一片内存，Next 就压在 T 的首 8 字节上
    struct FreeNode
    {
        FreeNode* Next;
    };

    // 每个线程只在自己私有的空闲链上 pop/push，共享链一律在 mutex_ 内读写。
    // 若允许多线程直接在共享链上 pop/push，「读 Next → CAS」这段窗口里链头被别的
    // 线程弹出、构造、再推回（ABA）就会让 CAS 把已在使用的对象发布成新链头。
    struct ThreadLocalCache
    {
        FreeNode* FreeListHead = nullptr;
        int FreeNodeCount = 0;
    };

#ifndef NDEBUG
    // 池只认自己发出去的指针：同一指针被归还两次，或把 new 出来的外来指针交给池，
    // 都只会先把空闲链写成自环（或把外部内存挂进链里），此后才以别处的越界写、
    // 串数据甚至崩溃现形——现场离根因很远。故在这一步就地拦下，并报出类型与指针。
    struct OwnedItemRegistry
    {
        std::mutex Mutex;
        std::unordered_set<T*> LiveItems;
    };
#endif

    // 槽位步长按空闲节点所需的对齐取整：T 的 sizeof 不是 alignof(FreeNode) 的整数倍时
    // （4 字节对齐的字段类型即如此），&newObjects[i] 会落在 8 字节对齐之外，节点指针即错位访问
    static constexpr size_t SlotByteCount = ObjectPoolDetail::SlotByteCountFor<T, FreeNode>;

    static_assert(sizeof(FreeNode) <= sizeof(T), "The T type is too small to hold the free list node!");
    static_assert(alignof(T) <= __STDCPP_DEFAULT_NEW_ALIGNMENT__, "The T type is over-aligned for the block allocation!");

    ObjectPool() : blockUnitNum_(64), blocks_(nullptr), sharedFreeList_(nullptr) {}
    ~ObjectPool()
    {
        Block* current = blocks_;
        while (current)
        {
            Block* next = current->Next;
            operator delete(current->Objects);
            delete current;
            current = next;
        }
    }
    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;

#ifndef NDEBUG
    static OwnedItemRegistry& GetOwnedItemRegistry()
    {
        static OwnedItemRegistry registry;
        return registry;
    }

    static void ReportOwnershipViolation(const char* violation, T* item)
    {
        fprintf(stderr, "ObjectPool<%s> %s: item:%p\n", typeid(T).name(), violation, static_cast<const void*>(item));
        assert(false && "ObjectPool ownership violation");
    }

    static void RegisterAllocatedItem(T* item)
    {
        bool wasNewlyHeld = false;
        {
            OwnedItemRegistry& registry = GetOwnedItemRegistry();
            std::lock_guard<std::mutex> guard(registry.Mutex);
            wasNewlyHeld = registry.LiveItems.insert(item).second;
        }
        if (!wasNewlyHeld)
        {
            ReportOwnershipViolation("Allocate handed out an item that is already held", item);
        }
    }

    // 摘牌与解构分成两步：先释放登记表的锁，再调析构——析构里若归还本池的另一个对象便不会自锁
    static void AssertAndUnregisterOwnedItem(T* item)
    {
        bool wasHeld = false;
        {
            OwnedItemRegistry& registry = GetOwnedItemRegistry();
            std::lock_guard<std::mutex> guard(registry.Mutex);
            wasHeld = registry.LiveItems.erase(item) == 1;
        }
        if (!wasHeld)
        {
            ReportOwnershipViolation("Deallocate got an item that is not currently held", item);
        }
    }
#endif

    // 本地链耗尽时调用：池内已无空闲节点则先申请一块，再从共享链摘一批到本地链
    void RefillThreadLocalFreeList()
    {
        std::lock_guard<std::mutex> guard(mutex_);
        if (sharedFreeList_ == nullptr)
        {
            AppendNewBlockToSharedListLocked();
        }

        FreeNode* batchHead = sharedFreeList_;
        FreeNode* batchTail = batchHead;
        int batchCount = 1;
        while (batchCount < blockUnitNum_ && batchTail->Next != nullptr)
        {
            batchTail = batchTail->Next;
            ++batchCount;
        }

        sharedFreeList_ = batchTail->Next;
        batchTail->Next = nullptr;
        threadLocalCache_.FreeListHead = batchHead;
        threadLocalCache_.FreeNodeCount = batchCount;
    }

    // 本地链积压超过一块时调用：整批交回共享链，使线程本地缓存的占用有上界
    void ReturnExcessThreadLocalNodesToSharedList()
    {
        std::lock_guard<std::mutex> guard(mutex_);
        const int returnCount = (threadLocalCache_.FreeNodeCount < blockUnitNum_) ? threadLocalCache_.FreeNodeCount : blockUnitNum_;

        FreeNode* returnHead = threadLocalCache_.FreeListHead;
        FreeNode* returnTail = returnHead;
        for (int i = 1; i < returnCount; ++i)
        {
            returnTail = returnTail->Next;
        }

        threadLocalCache_.FreeListHead = returnTail->Next;
        threadLocalCache_.FreeNodeCount -= returnCount;
        returnTail->Next = sharedFreeList_;
        sharedFreeList_ = returnHead;
    }

    void AppendNewBlockToSharedListLocked()
    {
        const size_t blockByteCount = static_cast<size_t>(blockUnitNum_) * SlotByteCount;
        if (blockByteCount / SlotByteCount != static_cast<size_t>(blockUnitNum_))
        {
            throw std::length_error("ObjectPool block byte count overflows");
        }

        char* const blockBytes = static_cast<char*>(operator new(blockByteCount));
        T* const newObjects = reinterpret_cast<T*>(blockBytes);
        try
        {
            blocks_ = new Block(newObjects, blocks_);
        }
        catch (...)
        {
            operator delete(blockBytes);
            throw;
        }

        FreeNode* newFreeList = nullptr;
        FreeNode* newFreeListTail = nullptr;
        for (int i = 0; i < blockUnitNum_; ++i)
        {
            FreeNode* node = reinterpret_cast<FreeNode*>(blockBytes + static_cast<size_t>(i) * SlotByteCount);
            node->Next = newFreeList;
            newFreeList = node;
            newFreeListTail = node;
        }

        // 循环逐个前插，故最后前插的那个正是这条新链的尾节点，把它接上原有共享链
        newFreeListTail->Next = sharedFreeList_;
        sharedFreeList_ = newFreeList;
    }

    int blockUnitNum_;
    std::mutex mutex_;
    Block* blocks_;
    FreeNode* sharedFreeList_;
    static thread_local ThreadLocalCache threadLocalCache_;
};

template <typename T>
thread_local typename ObjectPool<T>::ThreadLocalCache ObjectPool<T>::threadLocalCache_;

template <typename T>
T* Allocate()
{
    return ObjectPool<T>::GetInstance().Allocate();
}
template <typename T>
void Deallocate(T* ptr)
{
    ObjectPool<T>::GetInstance().Deallocate(ptr);
}
}
