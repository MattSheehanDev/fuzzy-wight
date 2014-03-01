#include "stdafx.h"
#include "ChatService.h"
#include "Server.h"

using namespace service;
using namespace io;


AuditService* AuditService::audit = nullptr;

//Main Service Function.
void CALLBACK AuditService::ServiceThread(PTP_CALLBACK_INSTANCE instance, PVOID context, PTP_WORK work)
{
	audit = static_cast<AuditService*>(context);

	//Start Server
	if (!audit->server.Startup())
	{
		audit->WriteEventLogEntry(L"WSAStartup failed", EVENTLOG_INFORMATION_TYPE);
		return;
	}

	//Create Listening Socket
	if (!audit->server.CreateListeningSocket())
	{
		audit->WriteEventLogEntry(L"Create Listening Socket failed", EVENTLOG_INFORMATION_TYPE);
		return;
	}
	
	//Run Server Loop
	audit->server.Run();

	//Signal stop event.
	SetEvent(audit->stop_event);
}


AuditService::AuditService(PWSTR serviceName, BOOL canStop, BOOL canShutdown, BOOL canPauseContinue) : ServiceBase(serviceName, canStop, canShutdown, canPauseContinue)
{
    stopping = FALSE;

	//Stop Event
	stop_event = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (stop_event == NULL)
        throw GetLastError();
}


AuditService::~AuditService(void)
{
	//Close Handle
    if (stop_event)
    {
        CloseHandle(stop_event);
        stop_event = NULL;
    }
}


//On Service Start
void AuditService::OnStart(DWORD consArgc, LPWSTR* consArgv)
{
	//Log Start
	WriteEventLogEntry(L"Service Start", EVENTLOG_INFORMATION_TYPE);

	//Service Runs on different thread
	work = CreateThreadpoolWork(&AuditService::ServiceThread, this, NULL);

	if (work == NULL)
	{
		WriteEventLogEntry(L"ThreadPool work failed", EVENTLOG_INFORMATION_TYPE);
		CloseThreadpoolWork(work);
	}
	
	SubmitThreadpoolWork(work);
}


//On Service Stop
void AuditService::OnStop()
{
    //Log Stop
    WriteEventLogEntry(L"Service Stop", EVENTLOG_INFORMATION_TYPE);

	//Signal Stopping
	stopping = TRUE;

	//Close Service
	audit->~AuditService();
	CloseThreadpoolWork(work);

    if (WaitForSingleObject(stop_event, INFINITE) != WAIT_OBJECT_0)
        throw GetLastError();
}