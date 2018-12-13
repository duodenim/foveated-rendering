#pragma once

#include <string>
#include <vector>

struct GVec2 {
  float x;
  float y;
};

struct GazePointDevice {
  std::string name;

  //Functions here - should probably be std::function
  void(*init_func)(void);
  const char * (*name_func)(void);
  void(*shutdown_func)(void);
  GVec2(*eye_position_func)(void);


  //Might need sdl library handle here (should just be void*)
  void* lib_file;
};


class GazePointManager {
public:
  static void InitDevices();

  static void FreeDevices();

  static void SelectDevice(const int index);

  static GVec2 GetGazePoint();

  static int GetDeviceCount();

  static std::string GetDeviceName(const int index);

  static int GetCurrentIndex();

private:
  static std::vector<GazePointDevice> m_Devices;
  static int m_DeviceIndex;
};