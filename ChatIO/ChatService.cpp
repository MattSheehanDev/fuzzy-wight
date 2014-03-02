#include "stdafx.h"
#include "ChatService.h"
#include "Server.h"

using namespace service;
using namespace io;


ChatService* ChatService::chat = nullptr;

//Main Service Function.
void CALLBACK ChatService::ServiceThread(PTP_CALLBACK_INSTANCE instance, PVOID context, PTP_WORK work)
{
	chat = static_cast<ChatService*>(context);

	//Start Server
	if (!chat->server.Startup())
	{
		chat->WriteEventLogEntry(L"WSAStartup failed", EVENTLOG_INFORMATION_TYPE);
		return;
	}

	//Create Listening Socket
	if (!chat->server.CreateListeningSocket())
	{
		chat->WriteEventLogEntry(L"Create Listening Socket failed", EVENTLOG_INFORMATION_TYPE);
		return;
	}
	
	//Run Server Loop
	chat->server.Run();

	//Signal stop event.
	//SetEvent(audit->stop_event);
}


ChatService::ChatService(PWSTR serviceName, BOOL canStop, BOOL canShutdown, BOOL canPauseContinue) : ServiceBase(serviceName, canStop, canShutdown, canPauseContinue)
{
    stopping = FALSE;

	//Stop Event
	stop_event = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (stop_event == NULL)
        throw GetLastError();
}


ChatService::~ChatService(void)
{
	//Close Handle
    if (stop_event)
    {
        CloseHandle(stop_event);
        stop_event = NULL;
    }
}


//On Service Start
void ChatService::OnStart(DWORD consArgc, LPWSTR* consArgv)
{
	//Log Start
	WriteEventLogEntry(L"Service Start", EVENTLOG_INFORMATION_TYPE);

	//Service Runs on different thread
	work = CreateThreadpoolWork(&ChatService::ServiceThread, this, NULL);

	if (work == NULL)
	{
		WriteEventLogEntry(L"ThreadPool work failed", EVENTLOG_INFORMATION_TYPE);
		CloseThreadpoolWork(work);
	}
	
	SubmitThreadpoolWork(work);
}


//On Service Stop
void ChatService::OnStop()
{
    //Log Stop
    WriteEventLogEntry(L"Service Stop", EVENTLOG_INFORMATION_TYPE);

	//Signal Stopping
	stopping = TRUE;
	SetEvent(stop_event);

    if (WaitForSingleObject(stop_event, INFINITE) != WAIT_OBJECT_0)
        throw GetLastError();

	//Close Service
	chat->~ChatService();
	CloseThreadpoolWork(work);
}