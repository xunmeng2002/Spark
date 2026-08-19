#include <Spark/Core/Platform/Platform.h>
#ifdef LINUX
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
