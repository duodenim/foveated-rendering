#ifndef FILE_LOADER_H
#define FILE_LOADER_H

#include <string>
#include <experimental/filesystem>

namespace FileLoader {
  const std::string GetFilePath(const std::string file);
  const std::experimental::filesystem::path GetRootPath();
};
#endif