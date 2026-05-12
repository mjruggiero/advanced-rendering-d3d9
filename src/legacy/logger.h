#pragma once

// Lightweight legacy logger used by the recovered MD3/Q3 code.
//
// The public macros are intentionally kept compatible with the old sample code:
//
//   LOG("message", Logger::LOG_DATA);
//   LOGFUNC("FunctionName()");
//
// The implementation has been cleaned up so each process run starts with a fresh
// log file instead of appending forever.

#include <fstream>
#include <mutex>
#include <string>

class Logger;
extern Logger logger;

#define LOG(message, logBits) \
    do { logger.sourceLine() = __LINE__; logger.sourceFile() = __FILE__; logger.logText((message), (logBits)); } while (0)

#define HEX(buffer, count, logBits) \
    do { logger.sourceLine() = __LINE__; logger.sourceFile() = __FILE__; logger.logHex((buffer), (count), (logBits)); } while (0)

#define RAW(message) \
    do { logger.sourceLine() = __LINE__; logger.sourceFile() = __FILE__; logger.logRaw((message)); } while (0)

#define INDENT(message, logBits) \
    do { logger.sourceLine() = __LINE__; logger.sourceFile() = __FILE__; logger.indent((message), (logBits)); } while (0)

#define UNDENT(message, logBits) \
    do { logger.sourceLine() = __LINE__; logger.sourceFile() = __FILE__; logger.undent((message), (logBits)); } while (0)

#define LOGBLOCK(name) \
    logger.sourceLine() = __LINE__; logger.sourceFile() = __FILE__; LogBlock __logBlock__(name)

#define LOGFUNC(name) \
    logger.sourceLine() = __LINE__; logger.sourceFile() = __FILE__; LogFlow __logFlow__(name)

class Logger
{
public:
    enum LogFlags
    {
        LOG_INDENT = 0x00000001,
        LOG_UNDENT = 0x00000002,
        LOG_FLOW = 0x00000004,
        LOG_BLOK = 0x00000008,
        LOG_DATA = 0x00000010,
        LOG_INFO = 0x00000012,
        LOG_WARN = 0x00000014,
        LOG_ERR = 0x00000018,
        LOG_CRIT = 0x00000020,
        LOG_ALL = 0xFFFFFFFF
    };

    explicit Logger(const std::string& filename);
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void start();
    void stop();
    void setLogFile(const std::string& filename);

    void logText(const std::string& message, LogFlags logBits = LOG_INFO);
    void logRaw(const std::string& message);
    void logHex(const char* buffer, unsigned int count, LogFlags logBits = LOG_INFO);
    void indent(const std::string& message, LogFlags logBits = LOG_INDENT);
    void undent(const std::string& message, LogFlags logBits = LOG_UNDENT);

    void operator+=(const std::string& message) { logText(message); }

    bool logStarted() const { return m_logStarted; }

    const bool& lineCharsFlag() const { return m_lineCharsFlag; }
    bool& lineCharsFlag() { return m_lineCharsFlag; }

    const unsigned int& logMask() const { return m_logMask; }
    unsigned int& logMask() { return m_logMask; }

    const std::string& logFile() const { return m_logFile; }
    std::string& logFile() { return m_logFile; }

    const unsigned int& sourceLine() const { return m_sourceLine; }
    unsigned int& sourceLine() { return m_sourceLine; }

    const std::string& sourceFile() const { return m_sourceFile; }
    std::string& sourceFile() { return m_sourceFile; }

private:
    std::string headerString(LogFlags logBits) const;
    const char* levelPrefix(LogFlags logBits) const;
    std::string currentTimestampForHeader() const;
    std::string currentTimestampForLine() const;
    void writeLineUnlocked(const std::string& line);
    bool shouldLog(LogFlags logBits) const;

    std::string m_logFile;
    std::string m_sourceFile;
    unsigned int m_sourceLine = 0;
    int m_indentCount = 0;
    int m_indentChars = 4;
    unsigned int m_logMask = LOG_ALL;
    bool m_logStarted = false;
    bool m_lineCharsFlag = false;

    std::ofstream m_stream;
    mutable std::mutex m_mutex;
};

class LogBlock
{
public:
    explicit LogBlock(const std::string& name)
        : m_name(name)
    {
        logger.indent("Begin block: " + m_name, Logger::LOG_INDENT);
    }

    ~LogBlock()
    {
        logger.undent("End block: " + m_name, Logger::LOG_UNDENT);
    }

private:
    std::string m_name;
};

class LogFlow
{
public:
    explicit LogFlow(const char* functionName)
        : m_functionName(functionName ? functionName : "")
    {
        logger.indent("Enter function: " + m_functionName, Logger::LOG_FLOW);
    }

    ~LogFlow()
    {
        logger.undent("Exit function: " + m_functionName, Logger::LOG_FLOW);
    }

private:
    std::string m_functionName;
};
