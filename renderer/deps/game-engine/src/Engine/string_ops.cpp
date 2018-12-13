#include "string_ops.h"
#include <sstream>
namespace StringOps {
  const std::string trim(const std::string &s) {
    std::stringstream stream(s);
    std::string out;
    stream >> out;
    return out;
  }

  const std::string removeEnd(const std::string &s) {
    for (int i = s.length() - 1; i > 0; i--) {
      if (s[i] == ' ' && s[i-1] != ' ') {
        return s.substr(0, i);
      }
    }
    return s;
  }
}

