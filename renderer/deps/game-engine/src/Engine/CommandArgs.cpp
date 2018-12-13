#include "CommandArgs.h"
#include "CommonTypes.h"
#include "Log.h"
#include "Config.h"
#include <cstring>

std::string CommandArgs::m_MapToLoad;

void CommandArgs::ParseArgs(int argc, char **argv) {
  //Check for --help
  if (argc == 2) {
    if (strcmp(argv[1], "--help") == 0) {
      Log::LogInfo("Usage:");
      Log::LogInfo("p3n path/to/map/file.map --Option=Value");
      Log::LogInfo("Options are any valid options in config.txt");
      exit(0);
    }
  }
  for (u32 i = 1; i < argc; i++) {
    //Check if this is a param or file load
    if (strncmp(argv[i], "--X", 2) == 0) {
      //Param to pass to Config
      std::string param(argv[i]);

      //Remove the --
      param = param.substr(2);

      //Find = and split the string
      std::string::size_type equalPosition = param.find('=');

      if (equalPosition == std::string::npos) {
        Log::LogFatal("BAD ARGUMENTS, USE --help");
        exit(1);
      } else {
        std::string optionName = param.substr(0, equalPosition);
        std::string optionValue = param.substr(equalPosition + 1);

        Config::AddOption(optionName, optionValue);
      }
    } else {
      //Assume that a parameter that doesn't begin with -- represents a file
      m_MapToLoad = std::string(argv[i]);
    }
  }
}
std::string CommandArgs::GetMapToLoad() {
  return m_MapToLoad;
}
