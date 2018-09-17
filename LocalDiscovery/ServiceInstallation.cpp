
#include <stdio.h>
#include <windows.h>
#include "ServiceInstaller.h"



void InstallService(PWSTR pszServiceName,
	PWSTR pszDisplayName,
	DWORD dwStartType,
	PWSTR pszDependencies,
	PWSTR pszAccount,
	PWSTR pszPassword)
{
	wchar_t szPath[MAX_PATH];
	SERVICE_STATUS ssSvcStatus = {};
	SC_HANDLE SCMManager = NULL;
	SC_HANDLE sService = NULL;


	//check null pointer error
	if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath)) ==0)
	{
		wprintf(L"GetModuleFileName failed w/err 0x%08lx\n", GetLastError());
		CleanUp(SCMManager, sService);
	}

	SCMManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE);
	if (SCMManager == NULL)
	{
		wprintf(L"OpenSCManager failed w/err 0x%08lx\n", GetLastError());
		CleanUp(SCMManager, sService);
	}

	//Install and Create Service
	sService = CreateService(SCMManager,
		pszServiceName,
		pszDisplayName,
		SERVICE_QUERY_STATUS,
		SERVICE_WIN32_OWN_PROCESS,
		dwStartType,
		SERVICE_ERROR_NORMAL,
		szPath,
		NULL,
		NULL,
		pszDependencies,
		pszAccount,
		pszPassword);

	if (sService == NULL)
	{
		wprintf(L"CreateService failed w/err 0x%08lx\n", GetLastError());
		CleanUp(SCMManager, sService);
	}

	wprintf(L"Service is installed");
}

SC_HANDLE CleanUp(SC_HANDLE param_SCMManager, SC_HANDLE param_sService)
{
	if (param_SCMManager)
	{
		CloseServiceHandle(param_SCMManager);
		//Null catcher
		param_SCMManager = NULL;
	}
	if (param_sService)
	{
		CloseServiceHandle(param_SCMManager);
		param_sService = NULL;
	}
	return param_SCMManager; //send a SC_HANDLE value
}

void UninstallService(PWSTR pszServiceName)
{
	SC_HANDLE SCMManager = NULL;
	SC_HANDLE sService = NULL;
	SERVICE_STATUS svcStatus = {};

	SCMManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (SCMManager == NULL)
	{
		wprintf(L"OpenSCManager failed error: 0x%08lx\n", GetLastError());
		CleanUp(SCMManager, sService);
	}

	//Stopping service
	if (ControlService(sService, SERVICE_CONTROL_STOP, &svcStatus))
	{
		wprintf(L"Stopping %s", pszServiceName);
		Sleep(2000);

		while (QueryServiceStatus(sService, &svcStatus))
		{
			if (svcStatus.dwCurrentState == SERVICE_STOP_PENDING)
			{
				wprintf(L".");
				Sleep(2000);
			}
			else break;
		}

		if (svcStatus.dwCurrentState == SERVICE_STOPPED)
		{
			wprintf(L"Service Stopped %s \n", sService);
		}
		else
		{
			wprintf(L"Service failed to stop %s \n", sService);
		}
	}

	//Remove the service by calling DeleteService
	if (!DeleteService(sService))
	{
		wprintf(L"DeleteService failed %s \n", sService);
		CleanUp(SCMManager, sService);
	}

}
