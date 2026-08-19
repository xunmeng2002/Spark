#pragma once
#ifdef WINDOWS
#include <Windows.h>


namespace spark::network
{
class IOCompletePort
{
public:
	IOCompletePort();
	~IOCompletePort();
	IOCompletePort(const IOCompletePort&) = delete;
	IOCompletePort& operator=(const IOCompletePort&) = delete;

	bool Create(int maxConcurrency = 0);
	bool Close();
	bool AssociateDevice(HANDLE device, ULONG_PTR completeKey);
	bool PostStatus(DWORD dwNumBytes, ULONG_PTR completeKey, OVERLAPPED* po = NULL);
	bool GetStatus(PDWORD pdwNumBytes, ULONG_PTR* pCompKey, OVERLAPPED** ppo, DWORD dwMilliseconds = INFINITE);


private:
	HANDLE m_Handle;
};
}
#endif // WINDOWS
