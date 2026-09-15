#pragma once
#include <list>
#include <mutex>
#include <condition_variable>


namespace Spark
{
template <typename T>
class ThreadSafeList
{
public:
	ThreadSafeList()
	{
	}
	ThreadSafeList(const ThreadSafeList& other)
	{
		std::lock_guard<std::mutex> guard(other.mutex_);
		items_ = other.items_;
	}
	ThreadSafeList& operator=(const ThreadSafeList&) = delete;

	void PushBack(T* item)
	{
		std::lock_guard<std::mutex> guard(mutex_);
		items_.push_back(item);
		conditionVariable_.notify_one();
	}
	T* PopFront()
	{
		std::unique_lock<std::mutex> lk(mutex_);
		conditionVariable_.wait(lk, [this] {return !items_.empty(); });
		T* item = items_.front();
		items_.pop_front();
		return item;
	}

private:
	mutable std::mutex mutex_;
	std::condition_variable conditionVariable_;
	std::list<T*> items_;
};
}
