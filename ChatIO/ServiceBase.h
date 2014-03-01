#pragma once

#include <windows.h>

namespace service
{
	class ServiceBase
	{
	public:
		ServiceBase(PWSTR serviceName, BOOL canStop = TRUE, BOOL canShutdown = TRUE, BOOL canPauseContinue = FALSE);
		virtual ~ServiceBase(void);

		static BOOL Run(ServiceBase &service);

		void Stop();

	protected:
		virtual void OnStart(DWORD consArgc, PWSTR* consArgv);
		virtual void OnStop();
		virtual void OnPause();
		virtual void OnContinue();
		virtual void OnShutdown();

		void SetStatus(DWORD currentState, DWORD exitCode = NO_ERROR, DWORD waitHint = 0);
		void WriteEventLogEntry(PWSTR message, WORD wType);

	private:
		static void WINAPI ServiceMain(DWORD conArgc, LPWSTR* consArgv);
		static void WINAPI ServiceControl(DWORD serviceControl);

		void Start(DWORD consArgc, PWSTR* consArgv);
		void Pause();
		void Continue();
		void Shutdown();

	private:
		static ServiceBase* service;

		PWSTR name;
		SERVICE_STATUS status;
		SERVICE_STATUS_HANDLE status_handle;
	};
}