#pragma once
#include <Spark/Network/NetworkExport.h>
#include <Spark/Network/Protocol/Head.h>
#include <string>
#include <unordered_map>


constexpr unsigned int StepHeaderLen = 36u;
constexpr unsigned int StepTailLen = 9u;
constexpr unsigned int SOH = 1u;

namespace spark::network
{
class NETWORK_EXPORTS StepUtility
{
public:
	static bool GetNextSoh(char* buff, int startIndex, int endIndex, int& sohIndex);
	static bool GetNextEqual(char* buff, int startIndex, int endIndex, int& equalIndex);
	static bool GetNext(char* buff, int startIndex, int endIndex, unsigned short& key, std::string& value, int& sohIndex);
	static bool GetFieldStart(char* buff, int startIndex, int endIndex, unsigned short& fieldID, int& fieldStartIndex);
	static bool GetFieldEnd(char* buff, int startIndex, int endIndex, unsigned short& fieldID, int& fieldEndIndex);
	static bool GetNextFieldZone(char* buff, int startIndex, int endIndex, unsigned short& fieldID, int& fieldStartIndex, int& fieldEndIndex);
	static bool GetPackageStart(char* buff, int startIndex, int endIndex, int& packageStartIndex);

	static void WriteString(char*& ppos, int key, bool value);
	static void WriteString(char*& ppos, int key, char value);
	static void WriteString(char*& ppos, int key, unsigned short value);
	static void WriteString(char*& ppos, int key, int value);
	static void WriteString(char*& ppos, int key, long long value);
	static void WriteString(char*& ppos, int key, double value);
	static void WriteString(char*& ppos, int key, std::string value);
	static void WriteString(char*& ppos, int key, char* value);
	template<typename T>
	static void WriteString(char*& ppos, int key, T value)
	{
		int len = sprintf(ppos, "%d=%s", key, value);
		ppos += len;
		*ppos++ = SOH;
	}
	static void WriteHexString(char*& ppos, int key, unsigned short value);


	static void HeadToStream(HeadField* head, char* buff, int size);
	static bool HeadFromStream(char* buff, int startIndex, int endIndex, HeadField* head);
	static void TailToStream(TailField* tail, char* buff, int size);
	static bool TailFromStream(char* buff, int startIndex, int endIndex, TailField* tail);

};
}
