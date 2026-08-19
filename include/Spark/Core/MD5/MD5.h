#pragma once
#include <Spark/Core/CoreExport.h>
#include <string>


namespace spark::core
{
std::string CORE_EXPORTS getMD5(const unsigned char* data, int len);
}
