#ifndef LOGGER_H
#define LOGGER_H

#include <functional>
#include <sstream>

namespace raver {

class Logger {
 public:
  enum LogLevel { Trace, Debug, Info, Warn, Error, Fatal, ENUM_LEVEL_NUM };

  using OutputCallback = std::function<void(std::string)>;

 public:
  Logger(const char* filename, int line, const char* func_name, int errornum,
         LogLevel);
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

inline Logger::LogLevel Logger::logLevel() { return g_loglevel; }

#define LOG_TRACE                                        \
  if (raver::Logger::logLevel() <= raver::Logger::Trace) \
  raver::Logger(__FILE__, __LINE__, __func__, 0, raver::Logger::Trace).stream()

#define LOG_DEBUG                                        \
  if (raver::Logger::logLevel() <= raver::Logger::Debug) \
  raver::Logger(__FILE__, __LINE__, __func__, 0, raver::Logger::Debug).stream()

#define LOG_INFO                                        \
  if (raver::Logger::logLevel() <= raver::Logger::Info) \
  raver::Logger(__FILE__, __LINE__, raver::Logger::Info).stream()

#define LOG_WARN raver::Logger(__FILE__, __LINE__, raver::Logger::Warn).stream()

#define LOG_ERROR \
  raver::Logger(__FILE__, __LINE__, raver::Logger::Error).stream()

#define LOG_FATAL \
  raver::Logger(__FILE__, __LINE__, raver::Logger::Fatal).stream()

#define LOG_SYSERR raver::Logger(__FILE__, __LINE__, false).stream()

#define LOG_SYSFATAL raver::Logger(__FILE__, __LINE__, true).stream()

}  // namespace raver

#endif
