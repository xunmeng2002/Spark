#include <Spark/Core/Utility/Utility.h>
#include <algorithm>
#include <format>
#include <iostream>
#include <cstring>
#include <fstream>


namespace spark::core
{
void Utility::ParseProcessName(const char* fullProcessName, char* processName, int len)
{
#if WINDOWS
	const char* temp = strrchr(fullProcessName, '\\');
	temp = temp == nullptr ? fullProcessName : temp + 1;
	// 用 strchr 找扩展名分隔符，避免 strtok 修改只读字符串
	const char* dot = strchr(temp, '.');
	if (dot != nullptr)
	{
		int name_len = static_cast<int>(dot - temp);
		int copy_len = (name_len < len - 1) ? name_len : len - 1;
		strncpy(processName, temp, copy_len);
		processName[copy_len] = '\0';
		return;
	}
#elif LINUX
	const char* temp = strrchr(fullProcessName, '/');
	temp = temp == nullptr ? fullProcessName : temp + 1;
#endif
	strncpy(processName, temp, len);
}


std::string Utility::ItoA(int value)
{
	return std::to_string(value);
}
std::string Utility::FtoA(double value)
{
	return std::format("{}", value);
}


void Utility::PrintBytes(const char* name, char* src, int len)
{
	std::ofstream ofs("temp.log", std::ios::app);
	std::string result;
	result += std::format("{}:", name);
	for (auto i = 0; i < len; ++i)
	{
		result += std::format("{:02X} ", static_cast<unsigned char>(src[i]));
	}
	ofs << result << std::endl;
}

void Utility::RemoveCharacter(char* src, int size, char c)
{
	auto read = src;
	auto write = src;
	for (auto i = 0; i < size; ++i, ++read)
	{
		if (*read != c)
		{
			*write = *read;
			++write;
		}
	}
}

ProtocolTypeType Utility::GetConfigProtocolType(const std::string& configProtocolType)
{
	if (configProtocolType == "step")
	{
		return ProtocolTypeType::Step;
	}
	else if (configProtocolType == "xtp")
	{
		return ProtocolTypeType::Xtp;
	}
	return ProtocolTypeType::Xtp;
}
}
