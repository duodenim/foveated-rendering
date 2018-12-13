#include "InputManager.h"
#include "Log.h"
#include <algorithm>

bool Input::mKeys[NUM_KEYS];

std::vector<Controller> Input::mControllers;
std::vector<GameObject *> Input::mListeners;
std::vector<std::vector<std::string>> Input::mGamepadVirtualInputs;
std::vector<std::vector<KeyboardVirtualInput>> Input::mKeyboardVirtualInputs;

bool Input::mShouldClose;

static int JoyAxesToIndex(JOYAXES axis) {
  switch (axis) {
    case JOYAXES::LEFT_X: {
      return 1;
    }
    case JOYAXES::LEFT_Y: {
      return 2;
    }
    case JOYAXES::RIGHT_X: {
      return 3;
    }
    case JOYAXES::RIGHT_Y: {
      return 4;
    }
    case JOYAXES::TRIGGER_L: {
      return 5;
    }
    case JOYAXES::TRIGGER_R: {
      return 6;
    }
  }
}

static JOYAXES IndexToJoyAxes(const int index) {
  switch (index) {
    case 1: {
      return JOYAXES::LEFT_X;
    }
    case 2: {
      return JOYAXES::LEFT_Y;
    }
    case 3: {
      return JOYAXES::RIGHT_X;
    }
    case 4: {
      return JOYAXES::RIGHT_Y;
    }
    case 5: {
      return JOYAXES::TRIGGER_L;
    }
    case 6: {
      return JOYAXES::TRIGGER_R;
    }
  }
}

static int JoyButtonsToIndex(JOYBUTTONS button) {
  switch(button) {
    case JOYBUTTONS::A: {
      return 0;
    }
    case JOYBUTTONS::B: {
      return 1;
    }
    case JOYBUTTONS::X: {
      return 2;
    }
    case JOYBUTTONS::Y: {
      return 3;
    }
    case JOYBUTTONS::SELECT: {
      return 4;
    }
    case JOYBUTTONS::START: {
      return 6;
    }
    case JOYBUTTONS::LEFT_JOY: {
      return 7;
    }
    case JOYBUTTONS::RIGHT_JOY: {
      return 8;
    }
    case JOYBUTTONS::LB: {
      return 9;
    }
    case JOYBUTTONS::RB: {
      return 10;
    }
    case JOYBUTTONS::DPAD_UP: {
      return 11;
    }
    case JOYBUTTONS::DPAD_DOWN: {
      return 12;
    }
    case JOYBUTTONS::DPAD_LEFT: {
      return 13;
    }
    case JOYBUTTONS::DPAD_RIGHT: {
      return 14;
    }
  }
}

static JOYBUTTONS IndexToJoyButtons(const int index) {
  switch(index) {
    case 0: {
      return JOYBUTTONS::A;
    }
    case 1: {
      return JOYBUTTONS::B;
    }
    case 2: {
      return JOYBUTTONS::X;
    }
    case 3: {
      return JOYBUTTONS::Y;
    }
    case 4: {
      return JOYBUTTONS::SELECT;
    }
    case 6: {
      return JOYBUTTONS::START;
    }
    case 7: {
      return JOYBUTTONS::LEFT_JOY;
    }
    case 8: {
      return JOYBUTTONS::RIGHT_JOY;
    }
    case 9: {
      return JOYBUTTONS::LB;
    }
    case 10: {
      return JOYBUTTONS::RB;
    }
    case 11: {
      return JOYBUTTONS::DPAD_UP;
    }
    case 12: {
      return JOYBUTTONS::DPAD_DOWN;
    }
    case 13: {
      return JOYBUTTONS::DPAD_LEFT;
    }
    case 14: {
      return JOYBUTTONS::DPAD_RIGHT;
    }
  }
}

void Input::Init() {
  //Reset key states
  for (int i = 0; i < NUM_KEYS; i++) {
    mKeys[i] = false;
  }

  mShouldClose = false;

  //Initialize virtual inputs
  //Virtual inputs work by mapping input indices to a string value
  //These will be loaded into the mVirtualInputs lookup table for fast access
  mKeyboardVirtualInputs.resize(NUM_KEYS);
  mGamepadVirtualInputs.resize(15 + 6); //15 for buttons, 6 for axes

  //Load some sane defaults
  mKeyboardVirtualInputs[SDL_SCANCODE_SPACE].push_back(KeyboardVirtualInput(CONFIRM_INPUT_NAME, 1.0f));
  mKeyboardVirtualInputs[SDL_SCANCODE_LSHIFT].push_back(KeyboardVirtualInput(BACK_INPUT_NAME, 1.0f));
  mKeyboardVirtualInputs[SDL_SCANCODE_UP].push_back(KeyboardVirtualInput(VERTICAL_INPUT_NAME, 1.0f));
  mKeyboardVirtualInputs[SDL_SCANCODE_DOWN].push_back(KeyboardVirtualInput(VERTICAL_INPUT_NAME, -1.0f));
  mKeyboardVirtualInputs[SDL_SCANCODE_LEFT].push_back(KeyboardVirtualInput(HORIZONTAL_INPUT_NAME, -1.0f));
  mKeyboardVirtualInputs[SDL_SCANCODE_RIGHT].push_back(KeyboardVirtualInput(HORIZONTAL_INPUT_NAME, 1.0f));
  mKeyboardVirtualInputs[SDL_SCANCODE_E].push_back(KeyboardVirtualInput(ENCOUNTER_INPUT_NAME, 1.0f));
  mKeyboardVirtualInputs[SDL_SCANCODE_C].push_back(KeyboardVirtualInput(CUTSCENE_INPUT_NAME, 1.0f));
  mKeyboardVirtualInputs[SDL_SCANCODE_R].push_back(KeyboardVirtualInput(DEBUG_VERTICAL_NAME, 1.0f));
  mKeyboardVirtualInputs[SDL_SCANCODE_F].push_back(KeyboardVirtualInput(DEBUG_VERTICAL_NAME, -1.0f));
  mGamepadVirtualInputs[JoyButtonsToIndex(JOYBUTTONS::A)].push_back(CONFIRM_INPUT_NAME);
  mGamepadVirtualInputs[JoyButtonsToIndex(JOYBUTTONS::B)].push_back(BACK_INPUT_NAME);
}

void Input::PollInputs() {
  SDL_Event e;
  while(SDL_PollEvent(&e)) {
    switch(e.type) {
      case SDL_QUIT: {
        mShouldClose = true;
        break;
      }
      case SDL_KEYDOWN: {
        if (!e.key.repeat) {
          mKeys[e.key.keysym.scancode] = true;
          for(const auto &input : mKeyboardVirtualInputs[e.key.keysym.scancode]) {
            InputCallbackData d;
            d.m_Name = input.m_Name;
            d.m_Value = input.m_Value;
            DispatchInput(d);
          }
        }
        break;
      }
      case SDL_KEYUP: {
        if (!e.key.repeat) {
          mKeys[e.key.keysym.scancode] = false;
          for(const auto &input : mKeyboardVirtualInputs[e.key.keysym.scancode]) {
            InputCallbackData d;
            d.m_Name = input.m_Name;
            d.m_Value = 0.0f;
            DispatchInput(d);
          }
        }
        break;
      }
      case SDL_CONTROLLERDEVICEADDED: {
        Controller c;
        c.mController = SDL_GameControllerOpen(e.cdevice.which);
        mControllers.push_back(c);
        break;
      }
      case SDL_CONTROLLERDEVICEREMOVED: {
        //TODO - Hook this up
      }
      case SDL_CONTROLLERAXISMOTION: {
        SDL_GameController* c = SDL_GameControllerFromInstanceID(e.caxis.which);
        for(int i = 0; i < mControllers.size(); i++) {
          if (mControllers[i].mController == c) {
            mControllers[i].mAxes[e.caxis.axis + 1] = (float)e.caxis.value / 32768;
            for (const auto &input : mGamepadVirtualInputs[15 + e.caxis.axis]) {
              InputCallbackData d;
              d.m_Value = (float)e.caxis.value / 32768;
              d.m_Name = input;
              DispatchInput(d);
            }
          }
        }
        break;
      }
      case SDL_CONTROLLERBUTTONDOWN:
      case SDL_CONTROLLERBUTTONUP: {
        SDL_GameController* c = SDL_GameControllerFromInstanceID(e.cbutton.which);
        for(int i = 0; i < mControllers.size(); i++) {
          if (mControllers[i].mController == c) {
            mControllers[i].mButtons[e.cbutton.button] = (e.cbutton.state == SDL_PRESSED);
            for (const auto &input : mGamepadVirtualInputs[e.cbutton.button]) {
              InputCallbackData d;
              d.m_Name = input;
              d.m_Value = e.cbutton.state == SDL_PRESSED ? 1.0f : 0.0f;
              DispatchInput(d);
            }
          }
        }
        break;
      }
    }
  }
}

bool Input::GetKeyState(int key) {
  return mKeys[key];
}

bool Input::GetClose() {
  return mShouldClose;
}

float Input::GetControllerAxisState(int index, JOYAXES axis) {
  if (index < mControllers.size()) {
    return mControllers[index].mAxes[JoyAxesToIndex(axis)];
  }
}

bool Input::GetControllerButtonState(int index, JOYBUTTONS button) {
  if (index < mControllers.size()) {
    return mControllers[index].mButtons[JoyButtonsToIndex(button)];
  }
  return false;
}

void Input::RegisterListener(GameObject *listener) {
  mListeners.push_back(listener);
}

void Input::RemoveListener(GameObject *listener) {
  auto it = std::find(mListeners.begin(), mListeners.end(), listener);
  if (it != mListeners.end()) {
    mListeners.erase(it);
  }
}

void Input::DispatchInput(const InputCallbackData &data) {
  for (int i = 0; i < mListeners.size(); i++) {
    mListeners[i]->Input(data);
  }
}

void Input::Shutdown() {
  for (int i = 0; i < mControllers.size(); i++) {
    SDL_GameControllerClose(mControllers[i].mController);
  }
  mControllers.clear();
}
