#ifndef LOG_H
#define LOG_H

#include <string>

class Log {
public:
  static void LogInfo(const std::string &message);
  static void LogWarning(const std::string &message);
  static void LogFatal(const std::string &message);
};

#endif //LOG_H
