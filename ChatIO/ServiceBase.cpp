#include "stdafx.h"
#include "ServiceBase.h"
#include <assert.h>
#include <strsafe.h>

using namespace service;


//Initialize service instance.
#pragma region Static

ServiceBase *ServiceBase::service = NULL;

//Runs Service
BOOL ServiceBase::Run(ServiceBase &s)
{
	service = &s;

	SERVICE_TABLE_ENTRY serviceTable[] = { { service->name, ServiceBase::ServiceMain }, { NULL, NULL } };

	return StartServiceCtrlDispatcher(serviceTable);
}


//Service Main
void WINAPI ServiceBase::ServiceMain(DWORD consArgc, PWSTR* consArgv)
{
	assert(service != NULL);

	//Register the handler function for the service
	service->status_handle = RegisterServiceCtrlHandler(service->name, ServiceBase::ServiceControl);

	if (service->status_handle == NULL)
		throw GetLastError();

	//Start service.
	service->Start(consArgc, consArgv);
}

//Service Control Handler
void WINAPI ServiceBase::ServiceControl(DWORD serviceControl)
{
	switch (serviceControl)
	{
	case SERVICE_CONTROL_STOP:
		service->Stop();
		break;
	case SERVICE_CONTROL_PAUSE:
		service->Pause();
		break;
	case SERVICE_CONTROL_CONTINUE:
		service->Continue();
		break;
	case SERVICE_CONTROL_SHUTDOWN:
		service->Shutdown();
		break;
	case SERVICE_CONTROL_INTERROGATE:
		break;
	default:
		break;
	}
}

#pragma endregion


ServiceBase::ServiceBase(PWSTR serviceName, BOOL canStop, BOOL canShutdown, BOOL canPauseContinue)
{
	//Service Name
	name = serviceName;

	status_handle = NULL;

	status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	status.dwCurrentState = SERVICE_START_PENDING;
	status.dwWin32ExitCode = NO_ERROR;
	status.dwServiceSpecificExitCode = 0;
	status.dwCheckPoint = 1;
	status.dwWaitHint = 0;

	//Service Commands
	status.dwControlsAccepted = 0;
	if (canStop) status.dwControlsAccepted |= SERVICE_ACCEPT_STOP;
	if (canShutdown) status.dwControlsAccepted |= SERVICE_ACCEPT_SHUTDOWN;
	if (canPauseContinue) status.dwControlsAccepted |= SERVICE_ACCEPT_PAUSE_CONTINUE;
}


ServiceBase::~ServiceBase(void) {}


#pragma region Service Control

void ServiceBase::Start(DWORD consArgc, PWSTR* consArgv)
{
    try
    {
		SetStatus(SERVICE_START_PENDING);

        //Service Start
        OnStart(consArgc, consArgv);

		SetStatus(SERVICE_RUNNING);
    }
    catch (...)
    {
        WriteEventLogEntry(L"Service failed to start.", EVENTLOG_ERROR_TYPE);
		SetStatus(SERVICE_STOPPED);
    }
}


void ServiceBase::Stop()
{
    try
    {
		SetStatus(SERVICE_STOP_PENDING);

        //Service Stop
        OnStop();

        SetStatus(SERVICE_STOPPED);
    }
    catch (...)
    {
        WriteEventLogEntry(L"Service failed to stop.", EVENTLOG_ERROR_TYPE);
		SetStatus(status.dwCurrentState);
    }
}


void ServiceBase::Pause()
{
    try
    {
		SetStatus(SERVICE_PAUSE_PENDING);

		//Server Pause
		OnPause();

		SetStatus(SERVICE_PAUSED);
    }
    catch (...)
    {
        WriteEventLogEntry(L"Service failed to pause.", EVENTLOG_ERROR_TYPE);
		SetStatus(SERVICE_RUNNING);
    }
}


void ServiceBase::Continue()
{
    try
    {
		SetStatus(SERVICE_CONTINUE_PENDING);

        //Server Continue
        OnContinue();

		SetStatus(SERVICE_RUNNING);
    }
    catch (...)
    {
        WriteEventLogEntry(L"Service failed to continue.", EVENTLOG_ERROR_TYPE);
		SetStatus(SERVICE_PAUSED);
    }
}


void ServiceBase::Shutdown()
{
    try
    {
        //Server Shutdown
        OnShutdown();

		SetStatus(SERVICE_STOPPED);
    }
    catch (...)
    {
        WriteEventLogEntry(L"Service failed to shutdown.", EVENTLOG_ERROR_TYPE);
    }
}


#pragma endregion



#pragma region Virtual Service Control


void ServiceBase::OnStart(DWORD consArgc, PWSTR* consArgv) {}
void ServiceBase::OnStop() {}
void ServiceBase::OnPause() {}
void ServiceBase::OnContinue() {}
void ServiceBase::OnShutdown() {}


#pragma endregion



void ServiceBase::SetStatus(DWORD currentState, DWORD exitCode, DWORD waitHint)
{
    //Service Status
    status.dwCurrentState = currentState;
    status.dwWin32ExitCode = exitCode;
    status.dwWaitHint = waitHint;
    (currentState == SERVICE_RUNNING || currentState == SERVICE_STOPPED) ? status.dwCheckPoint = 0 : status.dwCheckPoint++;

    //Report the status of the service to the SCM.
    SetServiceStatus(status_handle, &status);
}


//Writes to Event Log
void ServiceBase::WriteEventLogEntry(PWSTR message, WORD type)
{
    HANDLE eventSource = NULL;
    LPCWSTR strings[2] = { NULL, NULL };

    eventSource = RegisterEventSource(NULL, name);
    
	if (eventSource)
    {
        strings[0] = name;
        strings[1] = message;

        ReportEvent(eventSource, type, 0, 0, NULL, 2, 0, strings, NULL);

        DeregisterEventSource(eventSource);
    }
}
