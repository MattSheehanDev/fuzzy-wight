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
	Close();
}


//Creates user socket and initializes vars
BOOL Person::UserPrep(size_t num)
{
	if (!CreateSocketIO())
		return false;

	this->num = num;
	this->login = FALSE;

	ZeroMemory(packet.buffer, sizeof(packet.buffer));
	ZeroMemory(&(ol), sizeof(OVERLAPPED));
	packet.bytes_rcvd = 0;
	flags = 0;

	//Buffer
	packet.data_buffer.len = DATA_BUFSIZE;
	packet.data_buffer.buf = packet.buffer;

	return true;
}


//Send welcome message
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
	if(std::string(packet.buffer).find("\n") != std::string::npos)
	{
		if (!login)
		{
			if (!Login())
				return false;
		}
		else if (login)
		{
			if (!Message())
				return false;
		}
	}
	else
	{
		buffer += packet.buffer;

		if (!Receive())
			return false;
	}


	return true;
}


//Login the user
BOOL Person::Login()
{
	//Check if name exists
	if (Server::users.find(buffer) != Server::users.end())
	{
		//Create message string
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


//Send a message to everyone
BOOL Person::Message()
{
	//Create message string
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


//Broadcasts the user has left the room
BOOL Person::Logout()
{
	std::ostringstream oss;
	oss << name << " left the room.\r\n";

	std::string str = oss.str();
	wsabuf.buf = (CHAR*) str.c_str();
	wsabuf.len = strlen(str.c_str());

	FileManager::File.Write(wsabuf.buf, wsabuf.len);

	if (!Broadcast(TRUE))
		return false;

	ClearBuffer();

	return true;
}


//Broadcast message to everyone
BOOL Person::Broadcast(BOOL exceptMyself)
{
	for each(std::pair<std::string, Person*> pair in Server::users)
	{
		Person* user = pair.second;

		if (!exceptMyself || user->socket != this->socket)
			send(user->socket, wsabuf.buf, wsabuf.len, 0);
	}

	this->Receive();

	return true;
}


//Clear buffer
void Person::ClearBuffer()
{
	ZeroMemory(packet.buffer, sizeof(packet.buffer));
	packet.data_buffer.buf = "";
	packet.data_buffer.len = 0;

	buffer = "";
}