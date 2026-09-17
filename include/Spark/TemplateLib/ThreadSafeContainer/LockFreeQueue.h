#pragma once
#include <atomic>
#include <memory>


namespace Spark
{
template <typename T>
class LockFreeQueue
{
public:
    LockFreeQueue()
    {
        Node* dummy = new Node();
        head_.store(dummy, std::memory_order_release);
        tail_.store(dummy, std::memory_order_release);
    }
    LockFreeQueue(const LockFreeQueue&) = delete;
    LockFreeQueue& operator=(const LockFreeQueue&) = delete;
    ~LockFreeQueue()
    {
        while (Node* oldHead = head_.load(std::memory_order_acquire))
        {
            head_.store(oldHead->next, std::memory_order_release);
            delete oldHead;
		}
    }
    
    void PushBack(std::shared_ptr<T> data)
    {
        Node* newNode = new Node();
        Node* oldTail = tail_.load(std::memory_order_acquire);
		oldTail->data.swap(data);
		oldTail->next = newNode;
		tail_.store(newNode, std::memory_order_release);
    }
    std::shared_ptr<T> PopFront()
    {
        Node* oldHead = head_.load(std::memory_order_acquire);
        if (oldHead == tail_.load(std::memory_order_acquire))
        {
            return nullptr;
		}
		head_.store(oldHead->next, std::memory_order_release);
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
        return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire);
    }

private:
    struct Node
    {
        Node() : data(nullptr), next(nullptr) {}
        Node(std::shared_ptr<T>& val) : data(val), next(nullptr) {}

        std::shared_ptr<T> data;
        Node* next;
    };

    std::atomic<Node*> head_;
    std::atomic<Node*> tail_;
};
}

