#pragma once
#include <atomic>
#include <mutex>
#include <memory>


template <typename T>
class ObjectPool
{
private:
    struct Block
    {
        T* Objects;
        Block* Next;

        Block(T* objs, Block* next)
            : Objects(objs), Next(next)
        {
        }
    };

    struct FreeNode
    {
        std::atomic<FreeNode*> Next;
    };

    static_assert(sizeof(FreeNode) <= sizeof(T),
        "The T type is too small to hold the free list node!");

private:
    ObjectPool()
        : m_BlockUnitNum(64), m_Blocks(nullptr)
    {
    }

    ~ObjectPool()
    {
        Block* current = m_Blocks;
        while (current)
        {
            Block* next = current->Next;
            operator delete(current->Objects);
            delete current;
            current = next;
        }
    }

    ObjectPool(ObjectPool&) = delete;
    ObjectPool& operator=(ObjectPool&) = delete;

public:
    static ObjectPool& GetInstance()
    {
        static ObjectPool m_Instance;
        return m_Instance;
    }

    void SetBlockUnitNum(int blockUnitNum)
    {
        m_BlockUnitNum = blockUnitNum;
    }

    template<typename... Args>
    T* Allocate(Args&&... args)
    {
        while (true)
        {
            FreeNode* oldHead = m_FreeList.load(std::memory_order_acquire);
            FreeNode* nextNode = nullptr;
            if (oldHead != nullptr)
            {
                do {
                    nextNode = oldHead->Next.load(std::memory_order_acquire);
                } while (!m_FreeList.compare_exchange_weak(oldHead, nextNode, std::memory_order_release, std::memory_order_acquire) && oldHead != nullptr);
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
    template<typename... Args>
    std::shared_ptr<T> AllocateShared(Args&&... args)
    {
        T* obj = Allocate(std::forward<Args>(args)...);
        return std::shared_ptr<T>(obj, [](T* ptr) {
            ObjectPool<T>::GetInstance().Deallocate(ptr);
            });
    }
    void Deallocate(T* item)
    {
        if (item == nullptr) [[unlikely]]
            return;
        item->~T();
        FreeNode* node = reinterpret_cast<FreeNode*>(item);
        FreeNode* oldHead = m_FreeList.load(std::memory_order_acquire);
        do {
            node->Next.store(oldHead, std::memory_order_release);
        } while (!m_FreeList.compare_exchange_weak(oldHead, node, std::memory_order_release, std::memory_order_acquire));
    }

private:
    void Expand()
    {
        std::lock_guard<std::mutex> guard(m_Mutex);
        T* newObjects = static_cast<T*>(operator new(sizeof(T) * m_BlockUnitNum));
        try
        {
            Block* newBlock = new Block(newObjects, m_Blocks);
            m_Blocks = newBlock;
        }
        catch (...)
        {
            operator delete(newObjects);
            throw;
        }

        FreeNode* newFreeList = nullptr;
        for (int i = 0; i < m_BlockUnitNum; ++i)
        {
            FreeNode* node = reinterpret_cast<FreeNode*>(&newObjects[i]);
            node->Next.store(newFreeList, std::memory_order_relaxed);
            newFreeList = node;
        }

        FreeNode* oldHead = m_FreeList.load(std::memory_order_acquire);
        FreeNode* newHead = newFreeList;
        FreeNode* tail = reinterpret_cast<FreeNode*>(&newObjects[0]);
        do
        {
            tail->Next.store(oldHead, std::memory_order_relaxed);
        } while (!m_FreeList.compare_exchange_weak(oldHead, newHead, std::memory_order_release, std::memory_order_acquire));
    }

private:
    int m_BlockUnitNum;
    std::mutex m_Mutex;
    Block* m_Blocks;
    std::atomic<FreeNode*> m_FreeList = nullptr;
};

template<typename T>
T* Allocate()
{
    return ObjectPool<T>::GetInstance().Allocate();
}
