#include <Spark/Core/Platform/Dir.h>
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif
#ifdef __linux__
#include <sys/stat.h>
#include <unistd.h>
#endif // __linux__



namespace spark::core
{
bool Dir::IsDir(const char* path)
{
#ifdef _WIN32
	return _access(path, 00) == 0;
#elif __linux__
	return access(path, 00) == 0;
#else
	return false;
#endif
}
bool Dir::Create(const char* path, int mode)
{
#ifdef _WIN32
	return _mkdir(path) == 0;
#elif __linux__
	return mkdir(path, mode) == 0;
#else
	return false;
#endif
}
}

