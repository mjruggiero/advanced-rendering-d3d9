#pragma once

#include <string>

namespace Framework
{
    enum class LogLevel
    {
        Info,
        Warning,
        Error
    };

    class FrameworkLog
    {
    public:
        static void Write(LogLevel level, const char* message);
        static void WriteInfo(const char* message);
        static void WriteWarning(const char* message);
        static void WriteError(const char* message);

        static void WriteInfo(const std::string& message);
        static void WriteWarning(const std::string& message);
        static void WriteError(const std::string& message);
    };
}
