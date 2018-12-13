#include <Windows.h>
#include <algorithm>
#include <thread>
#include "GazePointDevice.h"

//Device for the mouse

GazePointDevice* GazePointDevice::instance = nullptr;

GazePointDevice::GazePointDevice()
{
}

GazePointDevice::~GazePointDevice()
{
}

GazePointDevice* GazePointDevice::getInstance() {
	if (instance == nullptr) {
		instance = new GazePointDevice();
	}
	return instance;
}


void GazePointDevice::run() {
	while (!getExitFlag()) {
		eye_position_mutex_.lock();
		POINT tmp_point;
		GetCursorPos(&tmp_point);
		vec2 tmp_vec;
		tmp_vec.x = std::fmax(0.0f, float(tmp_point.x) / float(GetSystemMetrics(SM_CXSCREEN)));
		tmp_vec.y = std::fmax(0.0f, float(tmp_point.y) / float(GetSystemMetrics(SM_CYSCREEN)));
		eye_position_ = tmp_vec;
		eye_position_mutex_.unlock();
	}
}

bool GazePointDevice::getExitFlag() {
	exit_flag_mutex_.lock();
	bool tmp_flag = exit_flag_;
	exit_flag_mutex_.unlock();
	return tmp_flag;
}

void GazePointDevice::setExitFlag() {
	exit_flag_mutex_.lock();
	exit_flag_ = true;
	exit_flag_mutex_.unlock();
}


void GazePointDevice::init() {
	position_update_thread_ = std::thread(&GazePointDevice::run, this);
}

void GazePointDevice::shutdown() {
	setExitFlag();
	position_update_thread_.join();
}

vec2 GazePointDevice::getEyePosition() {
	eye_position_mutex_.lock();
	vec2 tmp_vec = eye_position_;
	eye_position_mutex_.unlock();
	return tmp_vec;
}

void Eye_Init() {
	GazePointDevice::getInstance()->init();
}

EYE_TRACKER_API const char * Get_Device_Name()
{
	return "Mouse";
}

vec2 Get_Eye_Position() {
	return GazePointDevice::getInstance()->getEyePosition();
}

void Eye_Shutdown() {
	GazePointDevice::getInstance()->shutdown();
}