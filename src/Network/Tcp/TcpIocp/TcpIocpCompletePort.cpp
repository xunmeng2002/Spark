#include "Tcp/TcpIocp/TcpIocpCompletePort.h"

#ifdef _WIN32

namespace spark::network
{
IOCompletePort::IOCompletePort()
{
	m_Handle = NULL;
}
IOCompletePort::~IOCompletePort()
{
	Close();
}

bool IOCompletePort::Create(int maxConcurrency)
{
	m_Handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, maxConcurrency);
	return (m_Handle != NULL);
}
bool IOCompletePort::Close()
{
	BOOL result = TRUE;
	if (m_Handle != NULL)
	{
		result = CloseHandle(m_Handle);
		m_Handle = NULL;
	}
	return result;
}
bool IOCompletePort::AssociateDevice(HANDLE device, ULONG_PTR completeKey)
{
	return (CreateIoCompletionPort(device, m_Handle, completeKey, 0) == m_Handle);
}
bool IOCompletePort::PostStatus(DWORD dwNumBytes, ULONG_PTR completeKey, OVERLAPPED* po)
{
	return PostQueuedCompletionStatus(m_Handle, dwNumBytes, completeKey, po);
}
bool IOCompletePort::GetStatus(PDWORD pdwNumBytes, ULONG_PTR* pCompKey, OVERLAPPED** ppo, DWORD dwMilliseconds)
{
	return GetQueuedCompletionStatus(m_Handle, pdwNumBytes, pCompKey, ppo, dwMilliseconds);
}
}
#endif // _WIN32
