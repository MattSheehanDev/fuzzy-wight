#include "stdafx.h"
#include "FileManager.h"

using namespace files;


FileManager FileManager::File;


FileManager::FileManager() 
{
	//Initialize Critical Section
	cs = new CRITICAL_SECTION();
	InitializeCriticalSection(cs);

	//Fetch Date
	current_date = Date();

	//Open
	Open();
}

FileManager::~FileManager() 
{
	Close();
}


//Creates Audit directory if doesn't exist.
//Opens days file if exists, creates file otherwise.
void FileManager::Open()
{
	//For now just put in root
	SetCurrentDirectory(L"C:/");

	//Audit directory
	CreateDirectory(L"Audit", NULL);

	//File Name
	file_name = L"Audit/" + current_date + L".txt";

	//Create File Handle
	file = CreateFile(file_name.c_str(), GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL);
}


//Writes to file
void FileManager::Write(void* data, size_t packetsize)
{
	//Check if new day
	if (current_date != Date())
	{
		current_date = Date();
		Close();
		Open();
	}

	//Data
	CHAR* write = (CHAR*) data;

	//Synchronize
	EnterCriticalSection(cs);

	//Set Pointer to end of file
	SetFilePointer(file, 0, NULL, FILE_END);

	//Write
	WriteFile((HANDLE) file, write, packetsize, NULL, NULL);

	LeaveCriticalSection(cs);
}


//Closes file
void FileManager::Close()
{
	CloseHandle(file);
}


//Returns todays date.
std::wstring FileManager::Date()
{
	//Todays date
	time_t today = time(0);

	struct tm time_struct;
	wchar_t buf[80];

	//Initializes time_struct
	localtime_s(&time_struct, &today);

	//Formats date
	wcsftime(buf, sizeof(buf), L"%Y-%m-%d", &time_struct);

	return buf;
}