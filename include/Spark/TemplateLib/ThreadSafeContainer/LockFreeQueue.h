#pragma once
#include <atomic>
#include <memory>


namespace spark
{
template <typename T>
class LockFreeQueue
{
private:
    struct Node
    {
        Node() : data(nullptr), next(nullptr) {}
        Node(std::shared_ptr<T>& val) : data(val), next(nullptr) {}

        std::shared_ptr<T> data;
        Node* next;
    };
    std::atomic<Node*> m_Head;
    std::atomic<Node*> m_Tail;

public:
    LockFreeQueue()
    {
        Node* dummy = new Node();
        m_Head.store(dummy, std::memory_order_release);
        m_Tail.store(dummy, std::memory_order_release);
    }
    LockFreeQueue(const LockFreeQueue&) = delete;
    LockFreeQueue& operator=(const LockFreeQueue&) = delete;
    ~LockFreeQueue()
    {
        while (Node* oldHead = m_Head.load(std::memory_order_acquire))
        {
            m_Head.store(oldHead->next, std::memory_order_release);
            delete oldHead;
		}
    }
    
    void PushBack(std::shared_ptr<T> data)
    {
        Node* newNode = new Node();
        Node* oldTail = m_Tail.load(std::memory_order_acquire);
		oldTail->data.swap(data);
		oldTail->next = newNode;
		m_Tail.store(newNode, std::memory_order_release);
    }
    std::shared_ptr<T> PopFront()
    {
        Node* oldHead = m_Head.load(std::memory_order_acquire);
        if (oldHead == m_Tail.load(std::memory_order_acquire))
        {
            return nullptr;
		}
		m_Head.store(oldHead->next, std::memory_order_release);
        if (!oldHead)
        {
            return nullptr;
		}
		std::shared_ptr<T> result = oldHead->data;
		delete oldHead;
		return result;
    }
    bool Empty() const
    {
        return m_Head.load(std::memory_order_acquire) == m_Tail.load(std::memory_order_acquire);
    }
};
}

