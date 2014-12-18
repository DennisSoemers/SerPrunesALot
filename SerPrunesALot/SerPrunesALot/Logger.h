#pragma once

#include <fstream>
#include <inttypes.h>

#include "Options.h"
#include "StringBuilder.h"

#define NOMINMAX
#include <Windows.h>

#define ALLOW_LOGGING

#define LOG_MEMORY_USAGE() Logger::Instance().dumpMemoryUsage();
#define LOG_MESSAGE(message) Logger::Instance().log(message);
#define LOG_ERROR(message) Logger::Instance().logError(message);
#define LOG_SIZE_OF_PRIMITIVES() Logger::Instance().logSizeOfPrimitives();
#define LOG_SIZE_OF(type) Logger::Instance().logSizeOf(sizeof(type), #type);

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
		std::ofstream output("Logs\\SerPrunesALot.log", std::ios::ate | std::ios::app);
		output << message << std::endl;
		output.close();
#endif
	}

	inline void logError(const std::string &message)
	{
#ifdef ALLOW_LOGGING
		std::ofstream output("Logs\\SerPrunesALot_ERRORS.log", std::ios::ate | std::ios::app);
		output << message << std::endl;
		output.close();
#endif
	}

	inline void logSizeOf(size_t size, std::string type)
	{
#ifdef ALLOW_LOGGING
		std::ofstream output("Logs\\SerPrunesALot_SizeOf.log", std::ios::ate | std::ios::app);
		output << (StringBuilder() << "Size of " << type << " = " << size << " bytes").getString() << std::endl;
		output.close();
#endif
	}

	inline void logSizeOfPrimitives()
	{
#ifdef ALLOW_LOGGING
		std::ofstream output("Logs\\SerPrunesALot_SizeOf.log", std::ios::ate | std::ios::app);
		output << (StringBuilder() << "Size of int = " << sizeof(int) << " bytes").getString() << std::endl;
		output << (StringBuilder() << "Size of long = " << sizeof(long) << " bytes").getString() << std::endl;
		output << (StringBuilder() << "Size of long long = " << sizeof(long long) << " bytes").getString() << std::endl;
		output << (StringBuilder() << "Size of int32_t = " << sizeof(int32_t) << " bytes").getString() << std::endl;
		output << (StringBuilder() << "Size of int64_t = " << sizeof(int64_t) << " bytes").getString() << std::endl;
		output.close();
#endif
	}

private:
	Logger()
	{
#ifdef ALLOW_LOGGING
		std::ofstream output;
		output.open("Logs\\SerPrunesALot.log");
		output.close();

		std::ofstream outputErrors;
		outputErrors.open("Logs\\SerPrunesALot_ERRORS.log");
		outputErrors.close();

		std::ofstream outputSizeOf;
		outputErrors.open("Logs\\SerPrunesALot_SizeOf.log");
		outputErrors.close();
#endif
	}

	~Logger()	{}

	Logger(Logger const &);
	void operator=(Logger const &);
};