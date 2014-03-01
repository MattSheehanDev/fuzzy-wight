#include "stdafx.h"
#include "Person.h"
#include "Server.h"

#include <sstream>
#include <ppl.h>
#include <ppltasks.h>

using namespace io;
using namespace files;


Person::Person() : Socket() {}


Person::~Person() 
{
	//Close();
	//~LPSocket();
}


BOOL Person::UserPrep(size_t num)
{
	if (!CreateSocketIO())
		return false;

	this->num = num;
	this->login = FALSE;

	ZeroMemory(packet.buffer, sizeof(packet.buffer));
	packet.bytes_rcvd = 0;
	flags = 0;
	ZeroMemory(&(ol), sizeof(OVERLAPPED));

	//Buffer
	packet.data_buffer.len = DATA_BUFSIZE;
	packet.data_buffer.buf = packet.buffer;

	return true;
}


void Person::Welcome()
{
	//Create welcome string and prompt for name
	std::ostringstream oss;

	oss << "\r\n > Welcome to \033[96mMattChatt\033[39m! \r\n" <<
		" > " << num << " other people are connected at this time.\r\n" <<
		" > Please write your name and press Enter: ";

	std::string str = oss.str();

	wsabuf.buf = (CHAR*) str.c_str();
	wsabuf.len = strlen(str.c_str());

	//Send
	Send(wsabuf);

	ClearBuffer();
}


//Checks for errors, parses data, and posts another receive
BOOL WINAPI Person::IOThread()
{
	//Check for errors
	if (ErrorCheck())
		return false;

	//Enter pressed
	BOOL enter;
	std::string("\r\n") == std::string(packet.buffer) ? enter = true : enter = false;

	//if (buffer != "" && enter)
	//{
		if (!login && enter)
		{
			if (!Login())
				return false;
		}
		else if (login && enter)
		{
			if (!Message())
				return false;
		}
	//}
	else
	{
		buffer += packet.buffer;

		if (!Receive())
			return false;
	}


	return true;
}



BOOL Person::Login()
{
	//Check if name exists
	if (Server::users.find(buffer) != Server::users.end())
	{
		std::string str = "User Name is already in use. Pick Again: ";
		wsabuf.buf = (CHAR*) str.c_str();
		wsabuf.len = strlen(str.c_str());

		Send(wsabuf);

		ClearBuffer();

		return true;
	}

	//Set name
	name = buffer;

	//Clear
	ClearBuffer();

	//Set login
	login = TRUE;

	//Add to user array
	Server::users.insert(std::pair<std::string, Person*>(name, this));

	//Create message string
	std::ostringstream oss;
	oss << " > " << name << " joined the room.\r\n";

	std::string str = oss.str();
	wsabuf.buf = (CHAR*) str.c_str();
	wsabuf.len = strlen(str.c_str());

	//Write to file
	FileManager::File.Write(wsabuf.buf, wsabuf.len);

	//Broadcast to everyone (including yourself)
	if (!Broadcast())
		return false;

	return true;
}


BOOL Person::Message()
{
	std::ostringstream oss;
	oss << " > " << name << " : " << buffer << "\r\n";

	std::string str = oss.str();

	wsabuf.buf = (CHAR*) str.c_str();
	wsabuf.len = strlen(str.c_str());

	FileManager::File.Write(wsabuf.buf, wsabuf.len);

	if (!Broadcast(TRUE))
		return false;

	ClearBuffer();

	return true;
}


BOOL Person::Broadcast(BOOL exceptMyself)
{
	//concurrency::parallel_for_each(begin(Server::users), end(Server::users), [this, &exceptMyself](std::pair<SOCKET, Person*> pair)
	for each(std::pair<std::string, Person*> pair in Server::users)
	{
		Person* user = pair.second;

		if (!exceptMyself || user->socket != this->socket)
		{
			if (!user->Send(this->wsabuf))
			{
				if (WSAGetLastError() != ERROR_IO_PENDING)
					break;
			}
		}
	}

	//Receive more
	if (exceptMyself)
		this->Receive();

	return true;
}


WSABUF Person::Buffer(std::string str)
{
	WSABUF buf;

	buf.buf = (CHAR*)str.c_str();
	buf.len = strlen(str.c_str());

	return std::move(buf);
}


void Person::ClearBuffer()
{
	ZeroMemory(packet.buffer, sizeof(packet.buffer));
	packet.data_buffer.buf = "";
	packet.data_buffer.len = 0;

	buffer = "";
}