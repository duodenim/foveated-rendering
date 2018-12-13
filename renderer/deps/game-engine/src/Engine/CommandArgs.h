#pragma once
#include <string>

class CommandArgs {
public:
  static void ParseArgs(int argc, char** argv);
  static std::string GetMapToLoad();
private:
  static std::string m_MapToLoad;
};