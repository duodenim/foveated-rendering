#include "../EngineCore.h"
#include "InputComponent.h"
InputComponent::InputComponent() {
  mOwner = NULL;
}
InputComponent::~InputComponent() {
  if (mOwner) {
    Input::RemoveListener(mOwner);
  }
}
bool InputComponent::GetKeyState(int key) {
  return Input::GetKeyState(key);
}

float InputComponent::GetControllerAxis(int index, JOYAXES axis) {
  return Input::GetControllerAxisState(index, axis);
}
bool InputComponent::GetControllerButtonState(int index, JOYBUTTONS button) {
  return Input::GetControllerButtonState(index, button);
}
void InputComponent::RegisterCallback(GameObject *owner) {
  if (mOwner) {
    Input::RemoveListener(mOwner);
  }
  mOwner = owner;
  Input::RegisterListener(owner);
}
