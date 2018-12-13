#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H
#include <SDL_events.h>
#include <SDL_keyboard.h>
#include <vector>
#include "Controller.h"
#include "GameObject.h"
#include <functional>

const std::string CONFIRM_INPUT_NAME = "confirm";
const std::string BACK_INPUT_NAME = "back";
const std::string VERTICAL_INPUT_NAME = "vertical";
const std::string HORIZONTAL_INPUT_NAME = "horizontal";
const std::string ENCOUNTER_INPUT_NAME = "encounter";
const std::string CUTSCENE_INPUT_NAME = "cutscene";
const std::string DEBUG_VERTICAL_NAME = "dbg_vertical";

const int NUM_KEYS = 1024;

enum class INPUT_DEVICE_TYPE {
  KEYBOARD,
  MOUSE,
  GAMEPAD
};

struct KeyboardVirtualInput {
  std::string m_Name; //Name of the virtual input
  float m_Value; //Value to have when this key is pressed

  KeyboardVirtualInput(const std::string name, const float value):m_Name(name), m_Value(value) {}
};

struct InputCallbackData {
  std::string m_Name;
  float m_Value;
};

class Input {
public:
  static void Init();
  static void Shutdown();
  static void PollInputs();

  static bool GetKeyState(int key);
  static bool GetClose();
  static float GetControllerAxisState(int index, JOYAXES axis);
  static bool GetControllerButtonState(int index, JOYBUTTONS button);
  static void RegisterListener(GameObject* listener);
  static void RemoveListener(GameObject* listener);
  static void DispatchInput(const InputCallbackData &data);

private:
  static bool mKeys[NUM_KEYS];

  static std::vector<Controller> mControllers;
  static std::vector<GameObject *> mListeners;
  static std::vector<std::vector<std::string>> mGamepadVirtualInputs;
  static std::vector<std::vector<KeyboardVirtualInput>> mKeyboardVirtualInputs;

  static bool mShouldClose;
};

#endif
