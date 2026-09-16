#pragma once
#ifdef _WIN32
#include <Windows.h>


namespace Spark::Network
{
class IoCompletePort
{
public:
	IoCompletePort();
	~IoCompletePort();
	IoCompletePort(const IoCompletePort&) = delete;
	IoCompletePort& operator=(const IoCompletePort&) = delete;

	bool Create(int maxConcurrency = 0);
	bool Close();
	bool AssociateDevice(HANDLE device, ULONG_PTR completeKey);
	bool PostStatus(DWORD dwNumBytes, ULONG_PTR completeKey, OVERLAPPED* po = NULL);
	bool GetStatus(PDWORD pdwNumBytes, ULONG_PTR* pCompKey, OVERLAPPED** ppo, DWORD dwMilliseconds = INFINITE);


private:
	HANDLE handle_;
};
}
#endif // _WIN32
