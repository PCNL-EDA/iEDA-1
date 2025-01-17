#pragma once

#include <thread>

#include "LogLevel.hpp"
#include "RTU.hpp"

namespace irt {

using Loc = std::experimental::source_location;

#define LOG_INST (irt::Logger::getInst())

class Logger
{
 public:
  static void initInst();
  static Logger& getInst();
  static void destroyInst();
  // function
  irt_int getLogLevel() const { return _log_level; }
  void setLogLevel(const irt_int log_level) { _log_level = log_level; }
  void openLogFileStream(const std::string& log_file_path);
  void closeLogFileStream();
  void printLogFilePath();

  template <typename T, typename... Args>
  void info(Loc location, const T& value, const Args&... args)
  {
    if (_log_level > 0) {
      return;
    }
    printLog(LogLevel::kInfo, location, value, args...);
  }

  template <typename T, typename... Args>
  void warning(Loc location, const T& value, const Args&... args)
  {
    if (_log_level > 1) {
      return;
    }
    printLog(LogLevel::kWarning, location, value, args...);
  }

  template <typename T, typename... Args>
  void error(Loc location, const T& value, const Args&... args)
  {
    printLog(LogLevel::kError, location, value, args...);
    closeLogFileStream();
    exit(0);
  }

 private:
  // self
  static Logger* _log_instance;
  // config & database
  irt_int _log_level = 0;
  std::string _log_file_path;
  std::ofstream* _log_file = nullptr;
  size_t _temp_storage_size = 1024;
  std::vector<std::string> _temp_storage;

  Logger() { _temp_storage.reserve(_temp_storage_size); }
  Logger(const Logger& other) = delete;
  Logger(Logger&& other) = delete;
  ~Logger() { closeLogFileStream(); }
  Logger& operator=(const Logger& other) = delete;
  Logger& operator=(Logger&& other) = delete;
  // function
  template <typename T, typename... Args>
  void printLog(LogLevel log_level, Loc location, const T& value, const Args&... args)
  {
    const char* log_color_start;
    const char* log_level_char;

    switch (log_level) {
      case LogLevel::kInfo:
        log_color_start = "\033[1;34m";
        log_level_char = "Info";
        break;
      case LogLevel::kWarning:
        log_color_start = "\033[1;33m";
        log_level_char = "Warning";
        break;
      case LogLevel::kError:
        log_color_start = "\033[1;31m";
        log_level_char = "Error";
        break;
      default:
        log_color_start = "\033[1;32m";
        log_level_char = "Info";
        break;
    }
    const char* log_color_end = "\033[0m";

    std::string file_name = std::filesystem::absolute(getString(location.file_name(), ":", location.line()));
    file_name.erase(remove(file_name.begin(), file_name.end(), '\"'), file_name.end());
    if (log_level != LogLevel::kError) {
      std::string::size_type pos = file_name.find_last_of('/') + 1;
      file_name = file_name.substr(pos, file_name.length() - pos);
    }
    std::string header = getString("[RT ", getTimestamp(), " ", getCompressedBase62(std::stoul(getString(std::this_thread::get_id()))), " ",
                                   file_name, " ", location.function_name(), " ");
    std::string message = getString(value, args...);

    std::string origin_log = getString(header, log_level_char, "] ", message, "\n");
    std::string color_log = getString(header, log_color_start, log_level_char, log_color_end, "] ", message, "\n");

    if (_log_file != nullptr) {
      if (!_temp_storage.empty()) {
        for (std::string& origin_log : _temp_storage) {
          pushStream(*_log_file, origin_log);
        }
        _temp_storage.clear();
      }
      pushStream(*_log_file, origin_log);
    } else {
      if (_temp_storage.size() >= _temp_storage_size) {
        _temp_storage.clear();
      }
      _temp_storage.push_back(origin_log);
    }
    pushStream(std::cout, color_log);
  }

  template <typename T, typename... Args>
  std::string getString(T value, Args... args)
  {
    std::stringstream oss;
    pushStream(oss, value, args...);
    std::string string = oss.str();
    oss.clear();
    return string;
  }

  template <typename Stream, typename T, typename... Args>
  void pushStream(Stream& stream, T t, const Args&... args)
  {
    stream << t;
    pushStream(stream, args...);
  }

  template <typename Stream, typename T>
  void pushStream(Stream& stream, T t)
  {
    stream << t;
  }

  std::string getTimestamp()
  {
    std::string timestamp;

    time_t now = time(nullptr);
    tm* t = localtime(&now);
    char* buffer = new char[32];
    strftime(buffer, 32, "%Y%m%d %H:%M:%S", t);
    timestamp = buffer;
    delete[] buffer;
    buffer = nullptr;
    return timestamp;
  }

  std::string getCompressedBase62(uint64_t origin)
  {
    std::string base = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

    std::string result = "";
    while (origin != 0) {
      result.push_back(base[origin % base.size()]);
      origin /= base.size();
    }
    return result;
  }
};

}  // namespace irt
