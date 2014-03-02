#pragma once

#include "stdafx.h"
#include "Socket.h"
#include "Server.h"

#include <iostream>
#include <ppl.h>
#include <ppltasks.h>


using namespace io;
using namespace files;


#define _WIN32_WINNT 0x0600



//Creates Socket and properties
bool Socket::CreateSocketIO()
{
	//Create Overlapping Socket
	if ((socket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
		return false;

	//Create ThreadPool io
	if ((io = CreateThreadpoolIo((HANDLE) socket, Server::IoCompletionCallback, NULL, NULL)) == NULL)
		return false;

	return true;
}


//Checks io errors and bytes left
bool Socket::ErrorCheck()
{
	if (io_result != 0 || bytes_transferred == 0)
		return true;
	else
		return false;
}


//Post WSASend
bool Socket::Send(WSABUF buf)
{
	//Zero overlap
	ZeroMemory(&(ol), sizeof(OVERLAPPED));

	StartThreadpoolIo(io);

	//Send request
	if (WSASend(socket, &(buf), 1, NULL, 0, &ol, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			CancelThreadpoolIo(io);
			return false;
		}
	}
	return true;
}


//Post WSAReceive
bool Socket::Receive()
{
	//fill overlap with zeros
	ZeroMemory(&(ol), sizeof(OVERLAPPED));
	ZeroMemory(&packet.buffer, sizeof(packet.buffer));

	packet.data_buffer.len = DATA_BUFSIZE;
	packet.data_buffer.buf = packet.buffer;
	flags = 0;

	StartThreadpoolIo(io);

	//Receive request
	if (WSARecv(socket, &(packet.data_buffer), 1, NULL, &flags, &ol, NULL) == SOCKET_ERROR)
	{
		auto err = WSAGetLastError();

		if (err != ERROR_IO_PENDING)
		{
			CancelThreadpoolIo(io);
			return false;
		}
	}

	return true;
}

//Cancels/Closes threadpool io. Closes Socket.
void Socket::Close()
{
	printf("Socket %d closed\n", socket);
	closesocket(socket);
	CloseThreadpoolIo(io);
}