#ifndef INPUTCOMPONENT_H
#define INPUTCOMPONENT_H

#include "../InputManager.h"
#include "../Component.h"
#include <memory>

class InputComponent : public Component {
 public:
  InputComponent();
  ~InputComponent();
  bool GetKeyState(int key);
  float GetControllerAxis(int index, JOYAXES axis);
  bool GetControllerButtonState(int index, JOYBUTTONS button);
  void RegisterCallback(GameObject* owner);
 private:
  GameObject* mOwner;

};

#endif //INPUTCOMPONENT_H
