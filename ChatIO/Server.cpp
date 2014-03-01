#include "stdafx.h"
#include "Server.h"
//#include "ChatService.h"


using namespace io;
//using namespace service;

#define PORT 23


std::map<std::string, Person*> Server::users;


//Welcome Callback
void CALLBACK Server::WelcomeCallback(PTP_CALLBACK_INSTANCE, PVOID context, PVOID overlap, ULONG ioResult, ULONG_PTR bytesTransferred, PTP_IO io)
{
	Person* user = static_cast<Person*>(overlap);

	user->Welcome();
}


//IO Callback
void CALLBACK Server::IoCompletionCallback(PTP_CALLBACK_INSTANCE, PVOID context, PVOID overlap, ULONG ioResult, ULONG_PTR bytesTransferred, PTP_IO io)
{
	Person* user = static_cast<Person*>(overlap);
	user->bytes_transferred = bytesTransferred;
	user->io_result = ioResult;
	
	if (user->IOThread() == FALSE)
	{
		user->Close();
		Server::users.erase(user->name);
		delete user;
	}
}


Server::Server() {}

Server::~Server() 
{
	listener->Close();
}


//Setup
bool Server::Startup()
{
	//Startup. V2.2
	if ((WSAStartup(0x0202, &wsa_data)) != 0)
		return false;

	addr_len = sizeof(SOCKADDR_STORAGE) +16;

	return true;
}


//Create Socket to listen on
bool Server::CreateListeningSocket()
{
	//Create socket
	listener = new Socket();

	//Create Overlapping Socket
	if ((listener->socket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
		return false;

	//Create ThreadPool io
	if ((listener->io = CreateThreadpoolIo((HANDLE) listener->socket, Server::WelcomeCallback, NULL, NULL)) == NULL)
		return false;

	//Create Event
	if ((listener->hEvent = WSACreateEvent()) == WSA_INVALID_EVENT)
		return false;

	//Select Accept
	if (WSAEventSelect(listener->socket, listener->hEvent, FD_ACCEPT) != 0)
		return false;


	//if (!listener->CreateSocketIO())
	//	return false;

	//Server Properties
	sa_server.sin_family = AF_INET;
	sa_server.sin_addr.s_addr = htonl(INADDR_ANY);
	sa_server.sin_port = htons(PORT);


	//Bind socket to properties
	if((bind(listener->socket, (PSOCKADDR) &sa_server, sizeof(sa_server))) == INVALID_SOCKET)
		return false;

	if (listen(listener->socket, 5) == SOCKET_ERROR)
		return false;


	//Initialize AcceptEx
	GUID accept_guid = WSAID_ACCEPTEX;
	DWORD bytes;

	if ((WSAIoctl(listener->socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &accept_guid, sizeof (accept_guid), &acceptex, sizeof (acceptex), &bytes, NULL, NULL)) == SOCKET_ERROR)
		return false;

	return true;
}


//Runs continuous loop
void Server::Run()
{
	while (TRUE) 
	{
		//Create new user
		Person* user = new Person();

		if (!user->UserPrep(users.size()))
		{
			delete user;
			return;
		}

		StartThreadpoolIo(listener->io);

		if ((acceptex(listener->socket, user->socket, user->packet.data_buffer.buf, NULL, addr_len, addr_len, NULL, &user->ol)) == FALSE)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				user->Close();
				delete user;
				return;
			}
		}

		//Wait for Accept
		if (WSAWaitForMultipleEvents(1, &(listener->hEvent), FALSE, WSA_INFINITE, TRUE) != 0)
		{
			user->Close();
			delete user;
			return;
		}

		//Reset Event
		WSAEnumNetworkEvents(listener->socket, listener->hEvent, &network_events);

		//return true;

		//if (!Accept())
		//	continue;

		////Create a new user
		//Person* user = new Person();		

		//Set 
		//ZeroMemory(user->socket->packet.buffer, sizeof(user->socket->packet.buffer));
		//user->socket->packet.bytes_send = 0;

		//Accepts socket
		//if ((user->socket->socket = WSAAccept(listener->socket, &user->socket->addr, NULL, NULL, 0)) == SOCKET_ERROR)
		//{
		//	user->socket->Close();
		//	delete user;
		//	continue;
		//}


		//users.push_back(user);
		
		//Initially Welcome User
		//user->Welcome();
	}
}


