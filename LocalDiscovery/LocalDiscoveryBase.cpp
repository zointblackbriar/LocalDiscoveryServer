#pragma region Includes 
#include "LocalDiscoveryBase.h"
#include <assert.h>
#include <strsafe.h>
#pragma endregion

#pragma region Static Members

LocalDiscoveryBase *LocalDiscoveryBase::s_service = NULL;

//Run Service
BOOL LocalDiscoveryBase::Run(LocalDiscoveryBase &service)
{
	s_service = &service;

	SERVICE_TABLE_ENTRY serviceTable[] = { {service.m_name, ServiceMain}, {NULL, NULL} };

	return StartServiceCtrlDispatcher(serviceTable);
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
		WriteEventLogEntry(L"Serviec failed to start.", EVENTLOG_ERROR_TYPE);

		SetServiceStatus(SERVICE_STOPPED);
	}
}

void LocalDiscoveryBase::OnStart(DWORD dwArgc, PWSTR *pszArgv)
{

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

