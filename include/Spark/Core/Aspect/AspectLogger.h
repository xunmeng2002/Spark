#pragma once
#include <Spark/Core/CoreExport.h>


namespace Spark::Core
{
class CORE_EXPORTS AspectLogger
{
public:
    void Before(const char* funcName);
    void After(const char* funcName);
};
}

