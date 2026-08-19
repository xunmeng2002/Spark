#include <Spark/Core/Platform/Dir.h>
#ifdef WINDOWS
#include <direct.h>
#include <io.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif
#ifdef LINUX
#include <sys/stat.h>
#include <unistd.h>
#endif // LINUX



namespace spark::core
{
bool Dir::IsDir(const char* path)
{
#ifdef WINDOWS
	return _access(path, 00) == 0;
#elif LINUX
	return access(path, 00) == 0;
#else
	return false;
#endif
}
bool Dir::Create(const char* path, int mode)
{
#ifdef WINDOWS
	return _mkdir(path) == 0;
#elif LINUX
	return mkdir(path, mode) == 0;
#else
	return false;
#endif
}
}

