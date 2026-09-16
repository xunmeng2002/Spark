#include "Tcp/TcpIocp/TcpIocpCompletePort.h"

#ifdef _WIN32

namespace Spark::Network
{
IoCompletePort::IoCompletePort()
{
	handle_ = NULL;
}
IoCompletePort::~IoCompletePort()
{
	Close();
}

bool IoCompletePort::Create(int maxConcurrency)
{
	handle_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, maxConcurrency);
	return (handle_ != NULL);
}
bool IoCompletePort::Close()
{
	BOOL result = TRUE;
	if (handle_ != NULL)
	{
		result = CloseHandle(handle_);
		handle_ = NULL;
	}
	return result;
}
bool IoCompletePort::AssociateDevice(HANDLE device, ULONG_PTR completeKey)
{
	return (CreateIoCompletionPort(device, handle_, completeKey, 0) == handle_);
}
bool IoCompletePort::PostStatus(DWORD dwNumBytes, ULONG_PTR completeKey, OVERLAPPED* po)
{
	return PostQueuedCompletionStatus(handle_, dwNumBytes, completeKey, po);
}
bool IoCompletePort::GetStatus(PDWORD pdwNumBytes, ULONG_PTR* pCompKey, OVERLAPPED** ppo, DWORD dwMilliseconds)
{
	return GetQueuedCompletionStatus(handle_, pdwNumBytes, pCompKey, ppo, dwMilliseconds);
}
}
#endif // _WIN32
