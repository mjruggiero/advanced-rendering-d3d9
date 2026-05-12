#include "FrameworkLog.h"

#include <Windows.h>
#include <cstdio>

namespace Framework
{
	static const char* ToPrefix(LogLevel level)
	{
		switch (level)
		{
		case LogLevel::Info:
			return "[INFO] ";
		case LogLevel::Warning:
			return "[WARN] ";
		case LogLevel::Error:
			return "[ERROR] ";
		default:
			return "[LOG] ";
		}
	}

	void FrameworkLog::Write(LogLevel level, const char* message)
	{
		if (!message)
			return;

		char buffer[2048] = {};
		sprintf_s(buffer, "%s%s\n", ToPrefix(level), message);

		OutputDebugStringA(buffer);
	}

	void FrameworkLog::WriteInfo(const char* message)
	{
		Write(LogLevel::Info, message);
	}

	void FrameworkLog::WriteWarning(const char* message)
	{
		Write(LogLevel::Warning, message);
	}

	void FrameworkLog::WriteError(const char* message)
	{
		Write(LogLevel::Error, message);
	}

	void FrameworkLog::WriteInfo(const std::string& message)
	{
		WriteInfo(message.c_str());
	}

	void FrameworkLog::WriteWarning(const std::string& message)
	{
		WriteWarning(message.c_str());
	}

	void FrameworkLog::WriteError(const std::string& message)
	{
		WriteError(message.c_str());
	}
}
