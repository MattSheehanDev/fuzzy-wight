#pragma once

#include "Socket.h"
#include <string>


namespace io
{
	class Person : public Socket
	{
	public:
		Person();
		~Person();

		BOOL WINAPI IOThread();

		BOOL UserPrep(size_t num);
		void Welcome();

		BOOL Login();
		BOOL Message();
		BOOL Broadcast(BOOL exceptMyself = FALSE);

		WSABUF Buffer(std::string str);
		void ClearBuffer();

		//OVERLAPPED ol;
		//LPSocket socket;

		WSABUF wsabuf;
		std::string buffer;
		std::string name;
		DWORD num;
		BOOL login;
	};
}