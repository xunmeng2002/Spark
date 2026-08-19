#pragma once
#include <list>
#include <mutex>
#include <condition_variable>


namespace spark
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
		std::lock_guard<std::mutex> guard(other.m_Mutex);
		m_Items = other.m_Items;
	}
	ThreadSafeList& operator=(const ThreadSafeList&) = delete;

	void PushBack(T* item)
	{
		std::lock_guard<std::mutex> guard(m_Mutex);
		m_Items.push_back(item);
		m_ConditionVariable.notify_one();
	}
	T* PopFront()
	{
		std::unique_lock<std::mutex> lk(m_Mutex);
		m_ConditionVariable.wait(lk, [this] {return !m_Items.empty(); });
		T* item = m_Items.front();
		m_Items.pop_front();
		return item;
	}

private:
	mutable std::mutex m_Mutex;
	std::condition_variable m_ConditionVariable;
	std::list<T*> m_Items;
};
}
