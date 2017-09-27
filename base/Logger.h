#ifndef LOGGER_H
#define LOGGER_H

#include <functional>
#include <sstream>

namespace logging {

class Logger {
public:
    enum LogLevel {
        Trace, Debug, Info, Warn, Error, Fatal, ENUM_LEVEL_NUM
    };

    using OutputCallback = std::function<void (std::string)>;

public:
    Logger(const char* filename, int line, const char* func_name, int errornum, LogLevel);
    Logger(const char* filename, int line, LogLevel level);
    Logger(const char* filename, int line, bool to_abort);
    ~Logger();

    std::stringstream& stream();

    static LogLevel logLevel();
    static void setLevel(LogLevel level);

    void setOutputCallback(OutputCallback cb);

private:
    LogLevel level_;
    std::stringstream sstream_;
    static OutputCallback output_callback_;
};

extern Logger::LogLevel g_loglevel;

inline Logger::LogLevel Logger::logLevel()
{
    return g_loglevel;
}

#define LOG_TRACE if (logging::Logger::logLevel() <= logging::Logger::Trace) \
    logging::Logger(__FILE__, __LINE__, __func__, 0, logging::Logger::Trace).stream()

#define LOG_DEBUG if (logging::Logger::logLevel() <= logging::Logger::Debug) \
    logging::Logger(__FILE__, __LINE__, __func__, 0, logging::Logger::Debug).stream()

#define LOG_INFO if (logging::Logger::logLevel() <= logging::Logger::Info) \
    logging::Logger(__FILE__, __LINE__, logging::Logger::Info).stream()

#define LOG_WARN logging::Logger(__FILE__, __LINE__, logging::Logger::Warn).stream()

#define LOG_ERROR logging::Logger(__FILE__, __LINE__, logging::Logger::Error).stream()

#define LOG_FATAL logging::Logger(__FILE__, __LINE__, logging::Logger::Fatal).stream()

#define LOG_SYSERR logging::Logger(__FILE__, __LINE__, false).stream()

#define LOG_SYSFATAL logging::Logger(__FILE__, __LINE__, true).stream()

}

#endif
