#pragma once

#include <windows.h>

class LocalDiscoveryBase
{
public:
	//With the Run command, SCM will start (Service Control Manager)
	static BOOL Run(LocalDiscoveryBase &service);

	LocalDiscoveryBase(PWSTR pszServiceName,
		BOOL fCanStop = TRUE,
		BOOL fCanShutdown = TRUE,
		BOOL fCanPauseContinue = FALSE);

	//Destructor
	virtual ~LocalDiscoveryBase(void);

	//Stop the service
	void Stop();
protected:

	//Interface function starting
	virtual void OnStart(DWORD dwArgc, PWSTR *pszArgv);

	//Interface function of stopping
	virtual void OnStop();

	//Interface function of Pausing
	virtual void OnPause();

	//Interface function of shutting down
	virtual void OnShutdown();

	//Interface function of continuing 
	virtual void OnContinue();

	//Set the service status and report the status to the SCM
	void SetServiceStatus(DWORD dwCurrentState,
		DWORD dwWin32ExitCode = NO_ERROR,
		DWORD dwWaitHint = 0);

	//Write a Log to the Event Handler
	void WriteEventLogEntry(PWSTR pszMessage, WORD wType);

	//Write a Log with specified error type to the Event Handler
	void WriteErrorLogEntry(PWSTR pszFunction,
		DWORD dwError = GetLastError());
private:
	//Starting point of Service
	static void WINAPI ServiceMain(DWORD dwArgc, LPWSTR *lpszArgv);
	
	static void WINAPI ServiceCtrlHandler(DWORD dwCtrl);

	void Start(DWORD dwArgc, PWSTR *pszArgv);

	void Pause();

	void Continue();

	void Shutdown();

	static LocalDiscoveryBase *s_service;

	//The name of service
	PWSTR m_name;

	//The status of the service
	SERVICE_STATUS m_status;

	//The service status handle
	SERVICE_STATUS_HANDLE m_statusHandle;



};