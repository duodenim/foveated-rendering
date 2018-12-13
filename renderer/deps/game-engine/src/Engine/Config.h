#ifndef CONFIG_H
#define CONFIG_H
#include <string>
#include <map>
//Retrieves parameters from configuration files
namespace Config {
  void LoadFile(const std::string &file);
  int GetOptionInt(const std::string &option);
  std::string GetOptionString(const std::string &option);
  bool OptionExists(const std::string &option);
  void AddOption(const std::string &name, const std::string &value);
}
#endif