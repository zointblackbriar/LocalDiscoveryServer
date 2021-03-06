#pragma once

#include <windows.h>
//To install Service in Windows OS

void InstallService(PWSTR pszServiceName,
	PWSTR pszDisplayName,
	DWORD dwStartType,
	PWSTR pszDependencies,
	PWSTR pszAccount,
	PWSTR pszPassword);

void UninstallService(PWSTR pszServiceName);

SC_HANDLE CleanUp(SC_HANDLE param_SCMManager, SC_HANDLE param_sService);

