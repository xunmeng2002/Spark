#pragma once
#include <Spark/Core/CoreExport.h>
#ifdef __linux__
#include <errno.h>

namespace spark::core
{
int CORE_EXPORTS GetLastError();
int CORE_EXPORTS WSAGetLastError();
}
#endif

