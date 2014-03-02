#pragma once

#include "ServiceBase.h"
#include "Server.h"

namespace service
{
	class ChatService : public ServiceBase
	{
	public:
		ChatService(PWSTR serviceName, BOOL canStop = TRUE, BOOL canShutdown = TRUE, BOOL canPauseContinue = FALSE);
		virtual ~ChatService(void);

	protected:
		static void CALLBACK ServiceThread(PTP_CALLBACK_INSTANCE instance, PVOID context, PTP_WORK work);

		virtual void OnStart(DWORD consArgc, PWSTR* consArgv);
		virtual void OnStop();

	protected:
		static ChatService* chat;

		TP_WORK* work;

		io::Server server;
		BOOL stopping;
		HANDLE stop_event;
	};
}