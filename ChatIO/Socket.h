#pragma once

#include "FileManager.h"

#include <string>
#include <vector>

#define DATA_BUFSIZE 60000


namespace io
{
	typedef struct
	{
		CHAR buffer[DATA_BUFSIZE];
		WSABUF data_buffer;
		DWORD bytes_send;
		DWORD bytes_rcvd;

	} Packet, *LPPacket;


	class Socket
	{
	public:
		Socket(){};
		~Socket(){};

		BOOL WINAPI IOThread();

		bool CreateSocketIO();

		bool Send(WSABUF buf);
		bool Receive();

		void Close();
		bool ErrorCheck();


		OVERLAPPED ol;
		SOCKET socket;
		PTP_IO io;

		Packet packet;

		ULONG io_result;
		DWORD bytes_transferred;

		WSAEVENT hEvent;
		sockaddr addr;
		DWORD flags;

	};

}
