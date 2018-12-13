#include "FileLoader.h"
#include "EngineCore.h"
#include "Config.h"

#ifdef NDEBUG
  const bool USE_DEBUG_PATH = false;
#else
  const bool USE_DEBUG_PATH = true;
#endif

const std::string DATA_FOLDER_NAME = "data";

const std::string FileLoader::GetFilePath(const std::string file) {
  auto path = GetRootPath();
  path.append(file);
  return path.generic_string();
}

const std::experimental::filesystem::path FileLoader::GetRootPath() {
  if (Config::OptionExists("DataPath")) {
    return std::experimental::filesystem::path(Config::GetOptionString("DataPath"));
  } else {
    auto path = std::experimental::filesystem::current_path();
    if (USE_DEBUG_PATH) {
      path = path.parent_path();
    }
    path.append(DATA_FOLDER_NAME);
    return path;
  }

}
