#include "Config.h"
#include "Log.h"

#include <sstream>
#include <fstream>

static std::map<std::string, std::string> options;


/**
 * Fallback default options if config file cannot be found
 */
const std::map<std::string, std::string> defaultOptions = {
    std::pair<std::string, std::string>("Width", "1600"),
    std::pair<std::string, std::string>("Height", "900")
};
namespace Config {
  void LoadFile(const std::string &file) {
    //Store all config options from the file
    //Options are presented in the format: OPTIONNAME OPTIONVALUE
    std::ifstream fin;
    std::string option;
    std::string value;

    fin.open(file, std::ifstream::in);
    if (fin.fail()) {
      Log::LogWarning("CONFIG FILE NOT FOUND: " + file + "\nDefault Options will be used which may cause errors");
    } else {
      while (!fin.eof()) {
        fin >> option;
        fin >> value;
        options.insert(std::pair<std::string, std::string>(option, value));
      }
    }
    fin.close();
  }

  int GetOptionInt(const std::string &option) {
    return std::stoi(GetOptionString(option));
  }

  std::string GetOptionString(const std::string &option) {
    std::string output;

    auto it = options.find(option);
    if (it == options.end()) {
      auto defaultIt = defaultOptions.find(option);

      if (defaultIt == defaultOptions.end()) {
        Log::LogWarning("No option defined for " + option);
      } else {
        output = defaultIt->second;
      }
    } else {
      output = it->second;
    }

    return output;
  }

  bool OptionExists(const std::string &option) {
    auto it = options.find(option);

    return it != options.end();
  }

  void AddOption(const std::string &name, const std::string &value) {

    auto it = options.find(name);
    if (it != options.end()) {
      it->second = value;
    } else {
      options.insert(std::pair<std::string, std::string>(name, value));
    }
  }
}
