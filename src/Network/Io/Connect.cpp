#include <Spark/Network/Io/Connect.h>

using namespace std;

namespace Spark::Network
{
void Connect::PushBack(LinearBuffer<BufferSize>* buffer)
{
    lock_guard<mutex> guard(BuffersMutex);
    Buffers.push_back(buffer);
}
void Connect::PushFront(LinearBuffer<BufferSize>* buffer)
{
    lock_guard<mutex> guard(BuffersMutex);
    Buffers.push_front(buffer);
}
LinearBuffer<BufferSize>* Connect::GetNextBuffer()
{
    lock_guard<mutex> guard(BuffersMutex);
    if (Buffers.empty())
        return nullptr;
    auto buffer = Buffers.front();
    Buffers.pop_front();
    return buffer;
}
}
