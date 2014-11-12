#pragma once

#include <fstream>

#include "StringBuilder.h"

#define NOMINMAX
#include <Windows.h>

#define ALLOW_LOGGING

#define LOG_MEMORY_USAGE() Logger::Instance().dumpMemoryUsage();
#define LOG_MESSAGE(message) Logger::Instance().log(message);
#define LOG_ERROR(message) Logger::Instance().logError(message);

class Logger
{
public:
	// Returns the singleton Logger instance
	static Logger& Instance()
	{
		static Logger instance;
		return instance;
	}

	inline void dumpMemoryUsage()
	{
#ifdef ALLOW_LOGGING
		MEMORYSTATUSEX statex;
		statex.dwLength = sizeof(statex);
		GlobalMemoryStatusEx(&statex);

		LOG_MESSAGE("-------------------------------- Ser Prunes-A-Lot Memory Usage: --------------------------------")
		LOG_MESSAGE(StringBuilder() << "There is  " << statex.dwMemoryLoad << " percent of memory in use.")
		LOG_MESSAGE(StringBuilder() << "There are  " << statex.ullTotalPhys / 1024 << " total KB of physical memory.")
		LOG_MESSAGE(StringBuilder() << "There are  " << statex.ullAvailPhys / 1024 << " free  KB of physical memory.")
		LOG_MESSAGE(StringBuilder() << "There are  " << statex.ullTotalPageFile / 1024 << " total KB of paging file.")
		LOG_MESSAGE(StringBuilder() << "There are  " << statex.ullAvailPageFile / 1024 << " free  KB of paging file.")
		LOG_MESSAGE(StringBuilder() << "There are  " << statex.ullTotalVirtual / 1024 << " total KB of virtual memory.")
		LOG_MESSAGE(StringBuilder() << "There are  " << statex.ullAvailVirtual / 1024 << " free  KB of virtual memory.")
		LOG_MESSAGE(StringBuilder() << "There are  " << statex.ullAvailExtendedVirtual / 1024 << " free  KB of extended memory.")
		LOG_MESSAGE("-----------------------------------------------------------------------------------------")
#endif
	}

	inline void log(const std::string &message)
	{
#ifdef ALLOW_LOGGING
		std::ofstream output("SerPrunesALot.log", std::ios::ate | std::ios::app);
		output << message << std::endl;
		output.close();
#endif
	}

	inline void logError(const std::string &message)
	{
#ifdef ALLOW_LOGGING
		std::ofstream output("SerPrunesALotERRORS.log", std::ios::ate | std::ios::app);
		output << message << std::endl;
		output.close();
#endif
	}

private:
	Logger()
	{
#ifdef ALLOW_LOGGING
		std::ofstream output;
		output.open("SerPrunesALot.log");
		output.close();

		std::ofstream outputErrors;
		outputErrors.open("SerPrunesALotERRORS.log");
		outputErrors.close();
#endif
	}

	~Logger()	{}

	Logger(Logger const &);
	void operator=(Logger const &);
};