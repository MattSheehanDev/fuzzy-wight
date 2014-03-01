#pragma once

#include <iostream>
#include <fstream>
#include <time.h>
#include <string>

#include <ppl.h>


namespace files
{
	class FileManager
	{
	public:
		FileManager();
		~FileManager();

		static FileManager File;

		void Write(void* data, size_t packetsize);

	private:
		void Open();
		void Close();
		std::wstring Date();

	private:
		HANDLE file;
		LPCRITICAL_SECTION cs;
		std::wstring file_name;
		std::wstring current_date;
	};
}

