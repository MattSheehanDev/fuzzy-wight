#pragma once

#include "ServiceBase.h"
#include "Server.h"

namespace service
{
	class AuditService : public ServiceBase
	{
	public:
		AuditService(PWSTR serviceName, BOOL canStop = TRUE, BOOL canShutdown = TRUE, BOOL canPauseContinue = FALSE);
		virtual ~AuditService(void);

	protected:
		static void CALLBACK ServiceThread(PTP_CALLBACK_INSTANCE instance, PVOID context, PTP_WORK work);

		virtual void OnStart(DWORD consArgc, PWSTR* consArgv);
		virtual void OnStop();

	protected:
		static AuditService* audit;

		TP_WORK* work;

		io::Server server;
		BOOL stopping;
		HANDLE stop_event;
	};
}