// LocalDiscovery.cpp : Defines the entry point for the console application.
//

#pragma region Includes
#include "LocalDiscoveryService.h"
#include "LocalDiscoveryThreadPool.h"
#pragma endregion

LocalDiscoveryService::LocalDiscoveryService(PWSTR pszServiceName,
	BOOL fCanStop,
	BOOL fCanShutDown,
	BOOL fCanPauseContinue) : LocalDiscoveryBase(pszServiceName, fCanStop, fCanShutDown, fCanPauseContinue)
{
	m_fStopping = FALSE;

	//create an event for stopping case
	m_hStoppedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (m_hStoppedEvent == NULL)
	{
		throw GetLastError(); //Take the last error
	}

}

LocalDiscoveryService::~LocalDiscoveryService(void)
{
	//Destructor
	if (m_hStoppedEvent)
	{
		CloseHandle(m_hStoppedEvent);
		if (m_hStoppedEvent != NULL)
		{
			m_hStoppedEvent = NULL;
		}
	}
}

void LocalDiscoveryService::OnStart(DWORD dwArgc, LPWSTR *lpszArgv)
{
	//Write into Event Log
	WriteEventLogEntry(L"LocalDiscovery Service Start", EVENTLOG_INFORMATION_TYPE);

	//Queue the main service function for execution in a worker thread
	LocalDiscoveryThreadPool::QueueUserWorkItem(&LocalDiscoveryService::WorkerThreadofService, this);
}

void LocalDiscoveryService::WorkerThreadofService(void)
{
	while (!m_fStopping)
	{
		//Hold the thread two seconds
		::Sleep(2000);
	}

	SetEvent(m_hStoppedEvent);
}

void LocalDiscoveryService::OnStop()
{
	WriteEventLogEntry(L"LocalDiscoveryService is OnStop", EVENTLOG_INFORMATION_TYPE);

	m_fStopping = TRUE;
	if (WaitForSingleObject(m_hStoppedEvent, INFINITE) != WAIT_OBJECT_0)
	{
		throw GetLastError();
	}
}

