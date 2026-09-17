#pragma once
#include <atomic>
#include <mutex>
#include <memory>

namespace Spark
{
template <typename T> class ObjectPool
{
public:
    static ObjectPool& GetInstance()
    {
        static ObjectPool instance_;
        return instance_;
    }

    void SetBlockUnitNum(int blockUnitNum) { blockUnitNum_ = blockUnitNum; }

    template <typename... Args> T* Allocate(Args&&... args)
    {
        while (true)
        {
            FreeNode* oldHead = freeList_.load(std::memory_order_acquire);
            FreeNode* nextNode = nullptr;
            if (oldHead != nullptr)
            {
                do
                {
                    nextNode = oldHead->Next.load(std::memory_order_acquire);
                } while (!freeList_.compare_exchange_weak(oldHead, nextNode, std::memory_order_release, std::memory_order_acquire) &&
                         oldHead != nullptr);
                if (oldHead != nullptr)
                {
                    T* obj = reinterpret_cast<T*>(oldHead);
                    new (obj) T(std::forward<Args>(args)...);
                    return obj;
                }
            }
            Expand();
        }
    }
    template <typename... Args> std::shared_ptr<T> AllocateShared(Args&&... args)
    {
        T* obj = Allocate(std::forward<Args>(args)...);
        return std::shared_ptr<T>(obj, [](T* ptr) { ObjectPool<T>::GetInstance().Deallocate(ptr); });
    }
    void Deallocate(T* item)
    {
        if (item == nullptr) [[unlikely]]
            return;
        item->~T();
        FreeNode* node = reinterpret_cast<FreeNode*>(item);
        FreeNode* oldHead = freeList_.load(std::memory_order_acquire);
        do
        {
            node->Next.store(oldHead, std::memory_order_release);
        } while (!freeList_.compare_exchange_weak(oldHead, node, std::memory_order_release, std::memory_order_acquire));
    }

private:
    struct Block
    {
        Block(T* objs, Block* next) : Objects(objs), Next(next) {}

        T* Objects;
        Block* Next;
    };
    struct FreeNode
    {
        std::atomic<FreeNode*> Next;
    };

    static_assert(sizeof(FreeNode) <= sizeof(T), "The T type is too small to hold the free list node!");
    ObjectPool() : blockUnitNum_(64), blocks_(nullptr) {}
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
    void Expand()
    {
        std::lock_guard<std::mutex> guard(mutex_);
        T* newObjects = static_cast<T*>(operator new(sizeof(T) * blockUnitNum_));
        try
        {
            Block* newBlock = new Block(newObjects, blocks_);
            blocks_ = newBlock;
        }
        catch (...)
        {
            operator delete(newObjects);
            throw;
        }

        FreeNode* newFreeList = nullptr;
        for (int i = 0; i < blockUnitNum_; ++i)
        {
            FreeNode* node = reinterpret_cast<FreeNode*>(&newObjects[i]);
            node->Next.store(newFreeList, std::memory_order_relaxed);
            newFreeList = node;
        }

        FreeNode* oldHead = freeList_.load(std::memory_order_acquire);
        FreeNode* newHead = newFreeList;
        FreeNode* tail = reinterpret_cast<FreeNode*>(&newObjects[0]);
        do
        {
            tail->Next.store(oldHead, std::memory_order_relaxed);
        } while (!freeList_.compare_exchange_weak(oldHead, newHead, std::memory_order_release, std::memory_order_acquire));
    }

    int blockUnitNum_;
    std::mutex mutex_;
    Block* blocks_;
    std::atomic<FreeNode*> freeList_ = nullptr;
};

template <typename T> T* Allocate()
{
    return ObjectPool<T>::GetInstance().Allocate();
}
template <typename T> void Deallocate(T* ptr)
{
    ObjectPool<T>::GetInstance().Deallocate(ptr);
}
}
