#pragma once
#include <Spark/Core/CoreExport.h>

namespace Spark::Core
{
class CORE_EXPORTS Dir
{
public:
	static bool IsDir(const char* path);
	static bool Create(const char* path, int mode = 0777);
};
}


