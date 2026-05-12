#include "logger.h"

#include <algorithm>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <sstream>

Logger logger("app.log");

namespace
{
    std::tm LocalTime(std::time_t value)
    {
        std::tm result = {};

#if defined(_WIN32)
        localtime_s(&result, &value);
#else
        localtime_r(&value, &result);
#endif

        return result;
    }
}

Logger::Logger(const std::string& filename)
    : m_logFile(filename)
{
    start();
}

Logger::~Logger()
{
    stop();
}

void Logger::start()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_logStarted)
        return;

    const std::filesystem::path path(m_logFile);
    const std::filesystem::path parent = path.parent_path();

    if (!parent.empty())
        std::filesystem::create_directories(parent);

    // Important: truncate here so each application run starts with a fresh log.
    m_stream.open(m_logFile.c_str(), std::ios::out | std::ios::trunc);
    if (!m_stream.is_open())
        return;

    m_logStarted = true;
    m_indentCount = 0;

    m_stream << "---------------------------------------------- Log begins on "
             << currentTimestampForHeader()
             << " ----------------------------------------------" << std::endl;
}

void Logger::stop()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_logStarted)
        return;

    writeLineUnlocked("----------------------------------------------- Log ends on " +
        currentTimestampForHeader() +
        " -----------------------------------------------");

    m_stream.close();
    m_logStarted = false;
}


void Logger::setLogFile(const std::string& filename)
{
    if (filename.empty())
        return;

    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_logStarted)
    {
        writeLineUnlocked("----------------------------------------------- Log ends on " +
            currentTimestampForHeader() +
            " -----------------------------------------------");
        m_stream.close();
        m_logStarted = false;
    }

    m_logFile = filename;

    const std::filesystem::path path(m_logFile);
    const std::filesystem::path parent = path.parent_path();

    if (!parent.empty())
        std::filesystem::create_directories(parent);

    m_stream.open(m_logFile.c_str(), std::ios::out | std::ios::trunc);
    if (!m_stream.is_open())
        return;

    m_logStarted = true;
    m_indentCount = 0;

    m_stream << "---------------------------------------------- Log begins on "
        << currentTimestampForHeader()
        << " ----------------------------------------------" << std::endl;
}

void Logger::logText(const std::string& message, LogFlags logBits)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!shouldLog(logBits))
        return;

    writeLineUnlocked(headerString(logBits) + message);
}

void Logger::logRaw(const std::string& message)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_logStarted || !m_stream.is_open())
        return;

    m_stream << message;
    m_stream.flush();
}

void Logger::logHex(const char* buffer, unsigned int count, LogFlags logBits)
{
    if (!buffer)
        return;

    std::lock_guard<std::mutex> lock(m_mutex);

    if (!shouldLog(logBits))
        return;

    constexpr unsigned int bytesPerLine = 20;
    unsigned int logged = 0;

    while (logged < count)
    {
        std::ostringstream line;
        line << headerString(logBits);

        std::string ascii;
        ascii.reserve(bytesPerLine);

        for (unsigned int i = 0; i < bytesPerLine; ++i)
        {
            if (logged < count)
            {
                const unsigned char byte = static_cast<unsigned char>(buffer[logged++]);
                line << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
                     << static_cast<unsigned int>(byte) << ' ';

                ascii += (byte < 0x20 || byte > 0x7f) ? '.' : static_cast<char>(byte);
            }
            else
            {
                line << "-- ";
                ascii += '.';
            }
        }

        line << ascii;
        writeLineUnlocked(line.str());
    }
}

void Logger::indent(const std::string& message, LogFlags logBits)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!shouldLog(logBits))
        return;

    const char* marker = m_lineCharsFlag ? " \xDA " : " +- ";
    writeLineUnlocked(headerString(logBits) + marker + message);

    m_indentCount += m_indentChars;
}

void Logger::undent(const std::string& message, LogFlags logBits)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!shouldLog(logBits))
        return;

    m_indentCount -= m_indentChars;
    if (m_indentCount < 0)
        m_indentCount = 0;

    const char* marker = m_lineCharsFlag ? " \xC0 " : " +- ";
    writeLineUnlocked(headerString(logBits) + marker + message);
}

std::string Logger::headerString(LogFlags logBits) const
{
    std::ostringstream stream;
    stream << levelPrefix(logBits) << ' ';

    const std::size_t slash = m_sourceFile.find_last_of("\\/");
    const std::string filename =
        slash == std::string::npos ? m_sourceFile : m_sourceFile.substr(slash + 1);

    stream << std::setw(16) << std::right << filename
           << '[' << std::setw(4) << std::setfill('0') << m_sourceLine << std::setfill(' ') << ']'
           << currentTimestampForLine();

    const int safeIndent = std::clamp(m_indentCount, 0, 256);
    std::string indent(static_cast<std::size_t>(safeIndent), ' ');

    for (int i = 1; i < safeIndent; i += m_indentChars)
        indent[static_cast<std::size_t>(i)] = m_lineCharsFlag ? '\xB3' : '|';

    stream << indent;
    return stream.str();
}

const char* Logger::levelPrefix(LogFlags logBits) const
{
    switch (logBits)
    {
    case LOG_INDENT: return ">";
    case LOG_UNDENT: return "<";
    case LOG_ALL: return "A";
    case LOG_CRIT: return "!";
    case LOG_DATA: return "D";
    case LOG_ERR: return "E";
    case LOG_FLOW: return "F";
    case LOG_INFO: return "I";
    case LOG_WARN: return "W";
    default: return " ";
    }
}

std::string Logger::currentTimestampForHeader() const
{
    const std::time_t now = std::time(nullptr);
    const std::tm local = LocalTime(now);

    std::ostringstream stream;
    stream << std::put_time(&local, "%a %b %d %H:%M:%S %Y");
    return stream.str();
}

std::string Logger::currentTimestampForLine() const
{
    const std::time_t now = std::time(nullptr);
    const std::tm local = LocalTime(now);

    std::ostringstream stream;
    stream << std::put_time(&local, "%m/%d %H:%M ");
    return stream.str();
}

void Logger::writeLineUnlocked(const std::string& line)
{
    if (!m_logStarted || !m_stream.is_open())
        return;

    m_stream << line << std::endl;
}

bool Logger::shouldLog(LogFlags logBits) const
{
    return m_logStarted && m_stream.is_open() && ((logBits & m_logMask) != 0);
}
