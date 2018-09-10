#pragma once

#include "LocalDiscoveryBase.h"


class LocalDiscoveryService : public LocalDiscoveryBase
{
public: 

	LocalDiscoveryService(PWSTR pszServiceName,
		BOOL fCanStop = TRUE,
		BOOL fCanShutDown = TRUE,
		BOOL fCanPauseContinue = FALSE);
	virtual ~LocalDiscoveryService(void);

protected:
	virtual void OnStart(DWORD dwArgc, PWSTR *pszArgv);
	virtual void OnStop();

	//Worker Thread of Service
	void WorkerThreadofService(void);

private:
	BOOL m_fStopping;
	HANDLE m_hStoppedEvent;


};