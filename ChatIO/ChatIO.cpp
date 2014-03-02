//ChatIO.cpp

#include "stdafx.h"
#include "Server.h"

#include "ChatService.h"


#define SERVICE_NAME	L"auditio"

using namespace io;
using namespace service;


int main(int argc, TCHAR* argv[])
{
	ChatService service(SERVICE_NAME);
	if (!ChatService::Run(service))
	{
		auto err = GetLastError();
		wprintf(L"Service failed to run\n");
		return 1;
	}



	//Server server;

	////Start Server
	//if (!server.Startup())
	//{
	//	printf("Startup failed\n");
	//	return 1;
	//}

	////Create Listening Socket
	//if (!server.CreateListeningSocket())
	//{
	//	printf("CreateListeningSocket failed.\n");
	//	return 1;
	//}

	////Run Server Loop
	//server.Run();

	//return 0;
}