#include <Spark/Network/Io/Connect.h>

using namespace std;

namespace Spark::Network
{
void Connect::PushBack(Buffer<BufferSize>* buffer)
{
    lock_guard<mutex> guard(BuffersMutex);
    Buffers.push_back(buffer);
}
void Connect::PushFront(Buffer<BufferSize>* buffer)
{
    lock_guard<mutex> guard(BuffersMutex);
    Buffers.push_front(buffer);
}
Buffer<BufferSize>* Connect::GetNextBuffer()
{
    lock_guard<mutex> guard(BuffersMutex);
    if (Buffers.empty())
        return nullptr;
    auto buffer = Buffers.front();
    Buffers.pop_front();
    return buffer;
}
}
