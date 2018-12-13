#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <SDL_gamecontroller.h>

enum class CONTROLLER_ACTION_TYPE {
  JOYAXIS,
  JOYBUTTON
};

enum class JOYAXES {
  LEFT_X,
  LEFT_Y,
  RIGHT_X,
  RIGHT_Y,
  TRIGGER_L,
  TRIGGER_R
};

enum class JOYBUTTONS {
  A,
  B,
  X,
  Y,
  LB,
  RB,
  START,
  SELECT,
  LEFT_JOY,
  RIGHT_JOY,
  DPAD_UP,
  DPAD_DOWN,
  DPAD_LEFT,
  DPAD_RIGHT
};

class Controller {
public:
  Controller();
  SDL_GameController* mController;
  float mAxes[8];
  bool mButtons[16];
};

#endif
