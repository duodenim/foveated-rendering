#pragma once
#define EYE_TRACKER_API __declspec(dllexport)


#include <utility>
#include <mutex>

struct vec2 {
	float x;
	float y;
};

class GazePointDevice
{
public:
	GazePointDevice();
	~GazePointDevice();

	static GazePointDevice * getInstance();

	void init();
	void run();
	void shutdown();
	vec2 getEyePosition();

private:
	static GazePointDevice * instance;

	std::thread position_update_thread_;

	vec2 eye_position_;
	std::mutex eye_position_mutex_;

	bool getExitFlag();
	void setExitFlag();

	bool exit_flag_ = false;
	std::mutex exit_flag_mutex_;
};




//dllexport

extern "C" EYE_TRACKER_API void Eye_Init();

extern "C" EYE_TRACKER_API const char* Get_Device_Name();

extern "C" EYE_TRACKER_API vec2 Get_Eye_Position();

extern "C" EYE_TRACKER_API void Eye_Shutdown();