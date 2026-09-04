#include <Spark/Core/Platform/Platform.h>
#ifdef __linux__
#include <unistd.h>


namespace spark::core
{
int GetLastError()
{
    return errno;
}
int WSAGetLastError()
{
    return errno;
}
}
#endif
