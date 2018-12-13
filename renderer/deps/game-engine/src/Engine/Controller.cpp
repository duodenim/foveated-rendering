#include "Controller.h"

Controller::Controller() {
  for(int i = 0; i < 8; i++) {
    mAxes[i] = 0.0f;
  }

  for(int i = 0; i < 16; i++) {
    mButtons[i] = false;
  }
}