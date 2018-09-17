#pragma region Includes 
#include "LocalDiscoveryBase.h"
#include <assert.h>
#include <strsafe.h>
#pragma endregion


LocalDiscoveryBase *LocalDiscoveryBase::s_service = NULL;

//Run Service
BOOL LocalDiscoveryBase::Run(LocalDiscoveryBase &service)
{
	s_service = &service;

	SERVICE_TABLE_ENTRY serviceTable[] = { {service.m_name, ServiceMain}, {NULL, NULL} };

	return StartServiceCtrlDispatcher(serviceTable);
}

void WINAPI LocalDiscoveryBase::ServiceCtrlHandler(DWORD dwCtrl)
{
	switch (dwCtrl)
	{
		case SERVICE_CONTROL_STOP: s_service->Stop(); break;
		case SERVICE_CONTROL_PAUSE: s_service->Pause(); break;
		case SERVICE_CONTROL_CONTINUE: s_service->Continue(); break;
		case SERVICE_CONTROL_SHUTDOWN: s_service->Shutdown(); break;
		case SERVICE_CONTROL_INTERROGATE: break;
		default: break;
	}
}

void WINAPI LocalDiscoveryBase::ServiceMain(DWORD dwArgc, PWSTR *pszArgv)
{
	//assert(s_service != NULL);
	if (s_service == NULL)
		return;

	s_service->m_statusHandle = RegisterServiceCtrlHandler(s_service->m_name, ServiceCtrlHandler);

	if (s_service->m_statusHandle == NULL)
	{
		throw GetLastError();
	}

	s_service->Start(dwArgc, pszArgv);
}

LocalDiscoveryBase::LocalDiscoveryBase(PWSTR pszServiceName, BOOL fCanStop, BOOL fCanShutdown, BOOL fCanPauseContinue)
{
	m_statusHandle = NULL;

	m_name = (pszServiceName == NULL) ? L"" : pszServiceName;

	m_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;

	m_status.dwCurrentState = SERVICE_START_PENDING;

	//The accepted commands of service
	DWORD dwControlAccepted = 0;

	if (fCanStop)
		dwControlAccepted |= SERVICE_ACCEPT_STOP;
	if (fCanShutdown)
		dwControlAccepted |= SERVICE_ACCEPT_SHUTDOWN;
	if (fCanPauseContinue)
		dwControlAccepted |= SERVICE_ACCEPT_PAUSE_CONTINUE;
	m_status.dwControlsAccepted = dwControlAccepted;

	m_status.dwWin32ExitCode = NO_ERROR;
	m_status.dwServiceSpecificExitCode = 0;
	m_status.dwCheckPoint = 0;
	m_status.dwWaitHint = 0;
}

LocalDiscoveryBase::~LocalDiscoveryBase(void)
{
	//Destructor
}

void LocalDiscoveryBase::Shutdown()
{
	try {
		OnShutdown();
		SetServiceStatus(SERVICE_STOPPED);

	} 
	catch (DWORD dwError)
	{
		WriteErrorLogEntry(L"Service Stopped", dwError);
		SetServiceStatus(SERVICE_STOPPED, dwError);
	}
	catch (...)
	{
		WriteEventLogEntry(L"Service Force Shutdown.", EVENTLOG_ERROR_TYPE);
		SetServiceStatus(SERVICE_STOP);
	}
}

void LocalDiscoveryBase::Start(DWORD dwArgc, PWSTR *pszArgv)
{
	try {
		//SCM knows Service is starting
		SetServiceStatus(SERVICE_START_PENDING);

		//Perform service-specific initialization
		OnStart(dwArgc, pszArgv);

		//SCM knows service is started
		SetServiceStatus(SERVICE_RUNNING);

	}
	catch (DWORD dwError)
	{
		//Log the error 
		WriteErrorLogEntry(L"Service Start", dwError);

		//Set the service status to be stopped
		SetServiceStatus(SERVICE_STOPPED, dwError);
	}
	catch (...)
	{
		//Log the error
		WriteEventLogEntry(L"Service failed to start.", EVENTLOG_ERROR_TYPE);

		SetServiceStatus(SERVICE_STOP);
	}
}


void LocalDiscoveryBase::Stop()
{
	DWORD dwCurrentState = m_status.dwCurrentState;
	try {
		SetServiceStatus(SERVICE_STOP_PENDING);

		OnStop();

		SetServiceStatus(SERVICE_STOPPED);
	}
	catch (DWORD dwError)
	{
		//Log the error
		WriteErrorLogEntry(L"Service Stop", dwError);

		SetServiceStatus(dwCurrentState);
	}
	catch (...)
	{
		WriteErrorLogEntry(L"Service failed to stop", EVENTLOG_ERROR_TYPE);

		//Set the original service status.
		SetServiceStatus(dwCurrentState);
	}
}

void LocalDiscoveryBase::Pause()
{
	try {
		SetServiceStatus(SERVICE_PAUSE_PENDING);
		OnPause();
		SetServiceStatus(SERVICE_PAUSED); // Check SCM
	}
	catch (DWORD dwError)
	{
		//Log the Error into Windows Event Viewer
		WriteErrorLogEntry(L"Service Pause", dwError);

		SetServiceStatus(SERVICE_RUNNING);
	}
	catch (...)
	{
		WriteEventLogEntry(L"Service failed to pause", EVENTLOG_ERROR_TYPE);

		SetServiceStatus(SERVICE_RUNNING);
	}
}

void LocalDiscoveryBase::Continue()
{
	try
	{
		SetServiceStatus(SERVICE_CONTINUE_PENDING);
		OnContinue();
		SetServiceStatus(SERVICE_RUNNING);
	}
	catch (DWORD dwError)
	{
		WriteErrorLogEntry(L"Service continue", dwError);
		SetServiceStatus(SERVICE_PAUSED);
	}
	catch (...)
	{
		//Log the error
		WriteEventLogEntry(L"Service failed to resume", EVENTLOG_ERROR_TYPE);

		SetServiceStatus(SERVICE_PAUSED);
	}
}

//Function definition of SetServiceStatus
void LocalDiscoveryBase::SetServiceStatus(DWORD dwCurrentState,
										  DWORD dwWin32ExitCode,
										  DWORD dwWaitHint)
{
	static DWORD dwCheckPoint = 1;
	
	m_status.dwCurrentState = dwCurrentState;
	m_status.dwWin32ExitCode = dwWin32ExitCode;
	m_status.dwWaitHint = dwWaitHint;

	//Check status with regards to services
	m_status.dwCheckPoint =
		((dwCurrentState == SERVICE_RUNNING) ||
		(dwCurrentState == SERVICE_STOPPED)) ?
		0 : dwCheckPoint++;

	::SetServiceStatus(m_statusHandle, &m_status);
}

void LocalDiscoveryBase::WriteEventLogEntry(PWSTR pszMessage, WORD wType)
{
	HANDLE hEventSource = NULL;
	//Long Pointer Constant to Wide String i.e L"Hello world"
	LPCWSTR lpszStrings[2] = { NULL, NULL };

	hEventSource = RegisterEventSource(NULL, m_name);
	if(hEventSource)
	{
		lpszStrings[0] = m_name;
		lpszStrings[1] = pszMessage;

		ReportEvent(hEventSource,  // Event log handle
			wType,                 // Event type
			0,                     // Event category
			0,                     // Event identifier
			NULL,                  // No security identifier
			2,                     // Size of lpszStrings array
			0,                     // No binary data
			lpszStrings,           // Array of strings
			NULL                   // No binary data
		);

		DeregisterEventSource(hEventSource);
	}
}

void LocalDiscoveryBase::WriteErrorLogEntry(PWSTR pszFunction, DWORD dwError)
{
	wchar_t szMessage[260];
	StringCchPrintf(szMessage, ARRAYSIZE(szMessage),
		L"%s failed w/err 0x%08lx", pszFunction, dwError);
	WriteEventLogEntry(szMessage, EVENTLOG_ERROR_TYPE);
}
	

//Virtual Functions definition
void LocalDiscoveryBase::OnStart(DWORD dwArgc, PWSTR *pszArgv){}
void LocalDiscoveryBase::OnPause(){}
void LocalDiscoveryBase::OnShutdown(){}
void LocalDiscoveryBase::OnStop(){}
void LocalDiscoveryBase::OnContinue(){}
