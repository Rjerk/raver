#include "Logger.h"
#include <iostream>
#include <ctime>
#include <utility>
#include <cstring>

namespace logging {

Logger::LogLevel DEFAULT_LEVEL = Logger::Trace;

Logger::LogLevel g_loglevel = DEFAULT_LEVEL;

typename Logger::OutputCallback Logger::output_callback_ = [](std::string msg) {
    std::cout << msg << std::endl;
};

const char* level_string[] = { "Trace", "Debug", "Info ", "Warn ", "Error", "Fatal" };

std::string getTimeStr()
{
    std::time_t t;
    std::time(&t);
    std::string str{std::ctime(&t)};
    str.erase(str.size()-1);
    return str;
}

Logger::Logger(const char* filename, int line, const char* func_name,
               int errornum, LogLevel level)
    : level_(level)
{
    sstream_ << "[";
    sstream_ << level_string[level_];
    sstream_ << "]";
    sstream_ << getTimeStr() << " ";
    sstream_ << filename << " ";
    sstream_ << "(" << line << ") ";
    sstream_ << func_name << " ";
    sstream_ << ": ";
    if (errornum != 0) {
        sstream_ << ::strerror(errornum);
    }
}

Logger::Logger(const char* filename, int line, LogLevel level)
    : Logger(filename, line, "", 0, level)
{
}

Logger::Logger(const char* filename, int line, bool to_abort)
    : Logger(filename, line, "", errno, to_abort ? Fatal : Error)
{
}

Logger::~Logger()
{
    if (output_callback_) {
        std::string str = sstream_.str();
        sstream_.flush();
        output_callback_(str);
    }
    if (level_ == Fatal) {
        sstream_.flush();
        ::abort();
    }
}

void Logger::setLevel(Logger::LogLevel level)
{
    g_loglevel = level;
}

void Logger::setOutputCallback(OutputCallback cb)
{
    output_callback_ = cb;
}

std::stringstream& Logger::stream()
{
    return sstream_;
}

}
