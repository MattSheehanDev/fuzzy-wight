#pragma once

#include "Person.h"
#include <ppltasks.h>
#include <map>


namespace io
{
	class Server
	{
	public:
		Server();
		~Server();
		

		static void CALLBACK WelcomeCallback(PTP_CALLBACK_INSTANCE instance, PVOID context,
			PVOID overlap, ULONG ioResult, ULONG_PTR bytesTransferred, PTP_IO io);

		static void CALLBACK IoCompletionCallback(PTP_CALLBACK_INSTANCE instance, PVOID context, PVOID overlap, ULONG ioResult,
			ULONG_PTR bytesTransferred, PTP_IO io);


		static std::map<std::string, Person*> users;

		bool Startup();
		bool CreateListeningSocket();
		void Run();

	private:
		Socket* listener;
		LPFN_ACCEPTEX acceptex;

		WSADATA wsa_data;
		SOCKADDR_IN sa_server;

		DWORD addr_len;
		WSANETWORKEVENTS network_events;
	};
}
