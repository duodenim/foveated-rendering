#include "GazePoint.h"
#include <cmath>
#include <SDL_loadso.h>
#include "../../../Log.h"
#include <filesystem>

std::vector<GazePointDevice> GazePointManager::m_Devices;
int GazePointManager::m_DeviceIndex = 0;

const std::string TRACKER_FOLDER = "hardware";

void GazePointManager::InitDevices() {
  //Iterate over all dlls in the folder
  std::filesystem::directory_iterator folder(TRACKER_FOLDER);

  for (auto& file : folder) {
    if (file.path().extension().string() == ".dll") {
      //Call SDL functions to load libraries
      GazePointDevice device = {};
      device.lib_file = SDL_LoadObject(file.path().string().c_str());

      if (device.lib_file == nullptr) {
        Log::LogFatal("Could not load " + file.path().string());
        std::string error(SDL_GetError());
        Log::LogFatal(error);
      } else {
        //Load specified function names here and fill in structs
        device.init_func = (void(*)(void))SDL_LoadFunction(device.lib_file, "Eye_Init");

        if (device.init_func == nullptr) {
          Log::LogFatal("Could not load init function");
        }

        device.shutdown_func = (void(*)(void))SDL_LoadFunction(device.lib_file, "Eye_Shutdown");

        if (device.shutdown_func == nullptr) {
          Log::LogFatal("Could not load shutdown function");
        }

        device.name_func = (const char * (*) (void))SDL_LoadFunction(device.lib_file, "Get_Device_Name");

        if (device.name_func == nullptr) {
          Log::LogFatal("Could not load name function");
        }

        device.name = std::string(device.name_func());

        Log::LogInfo("Loaded device: " + device.name);

        device.eye_position_func = (GVec2(*) (void))SDL_LoadFunction(device.lib_file, "Get_Eye_Position");

        if (device.eye_position_func == nullptr) {
          Log::LogFatal("Could not load eye position function");
        }

        m_Devices.push_back(device);
      }
    }
  }
  
}

void GazePointManager::FreeDevices() {
  m_Devices[m_DeviceIndex].shutdown_func();
  for (auto& device : m_Devices) {
    SDL_UnloadObject(device.lib_file);
  }
}

void GazePointManager::SelectDevice(const int index) {
  static bool firstRun = true;

  if (!firstRun) {
    m_Devices[m_DeviceIndex].shutdown_func();
  }
  m_DeviceIndex = index;
  m_Devices[index].init_func();
  firstRun = false;
}

GVec2 GazePointManager::GetGazePoint() {
  return m_Devices[m_DeviceIndex].eye_position_func();
}

int GazePointManager::GetDeviceCount() {
  return m_Devices.size();
}

std::string GazePointManager::GetDeviceName(const int index) {
  return m_Devices[index].name;
}

int GazePointManager::GetCurrentIndex() {
  return m_DeviceIndex;
}
