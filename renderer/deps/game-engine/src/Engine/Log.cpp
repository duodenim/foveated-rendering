#include "Log.h"

#include <iostream>

#define FATAL_COLOR "\033[31m"
#define WARNING_COLOR "\033[33m"
#define RESET_COLOR "\033[0m"

void Log::LogInfo(const std::string &message) {
  std::cout << message << std::endl;
}

void Log::LogWarning(const std::string &message) {
  std::cout << WARNING_COLOR << "WARNING: " << message << RESET_COLOR << std::endl;
}

void Log::LogFatal(const std::string &message) {
  std::cout << FATAL_COLOR << "FATAL: " << message << RESET_COLOR << std::endl;
}
