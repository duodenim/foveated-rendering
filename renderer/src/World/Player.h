#ifndef PLAYER_H
#define PLAYER_H

#include <GameObject.h>
#include <Components/CameraComponent.h>
#include <Components/InputComponent.h>

/**
 * This class is responsible for handling the player's movement and interaction
 * with the overworld
 */

class WindowComponent;

class Player : public GameObject {
public:
  Player();
  ~Player();
  /**
   * Inherited GameObject Functions
   */
  void Update(float deltaTime);
  void Input(const InputCallbackData &inputData);

private:
  CameraComponent mCamera;
  InputComponent mInput;

  /**
   * Input variables
   */
  float inputDirection;
  float m_InputFreeYMovement;
  float viewAngle;
  float throttle;

  Mat4 mCameraData;
};

#endif //PLAYER_H
