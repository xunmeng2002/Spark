#include <Spark/Network/IO/Connect.h>

using namespace std;

namespace spark::network
{
void Connect::PushBack(Buffer<BuffSize>* buffer)
{
	lock_guard<mutex> guard(BuffersMutex);
	Buffers.push_back(buffer);
}
void Connect::PushFront(Buffer<BuffSize>* buffer)
{
	lock_guard<mutex> guard(BuffersMutex);
	Buffers.push_front(buffer);
}
Buffer<BuffSize>* Connect::GetNextBuffer()
{
	lock_guard<mutex> guard(BuffersMutex);
	if (Buffers.empty())
		return nullptr;
	auto buffer = Buffers.front();
	Buffers.pop_front();
	return buffer;
}
}
