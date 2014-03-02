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
		BOOL Logout();

		BOOL Broadcast(BOOL exceptMyself = FALSE);
		void ClearBuffer();

		WSABUF wsabuf;
		std::string buffer;
		std::string name;
		DWORD num;
		BOOL login;
	};
}