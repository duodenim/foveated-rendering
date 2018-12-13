#include "Player.h"
#include <Log.h>
#include <EngineCore.h>
#include <Config.h>
#include "../UI/WindowComponent.h"

const float VIEW_Y = 0.0f;
const float WALK_SPEED = 1.5f;

#include <imgui.h>
Player::Player() {

  mCameraData = glm::mat4(0.0f);
  mCamera.SetPosition(Vec3(0.0f, VIEW_Y, 0.0f));
  mCamera.SetFOV(65 * M_PI / 180.0f);
  mCamera.SetScreenShader("camera.vert", "camera.frag");
  mCamera.SetUserData(mCameraData);
  mCamera.SetAsMainCamera();
  inputDirection = 0;
  viewAngle = -M_PI / 2;
  mInput.RegisterCallback(this);
  throttle = 0;

  m_InputFreeYMovement = 0.0f;
}

Player::~Player() {

}

void Player::Update(float deltaTime) {
  static float time = 0.0f;
  static u32 numFrames = 0;
  static float shownFrameRate = 0.0f;
  static float runningFrameRateTotal = 0.0f;


  //Average framerate each second
  time += deltaTime;
  numFrames++;
  runningFrameRateTotal += (1 / deltaTime);

  if (time >= 1.0f) {
    time = 0.0f;
    shownFrameRate = runningFrameRateTotal / numFrames;
    runningFrameRateTotal = 0.0f;
    numFrames = 0;
  }

  mCamera.SetUserData(mCameraData);
  viewAngle += (inputDirection) * deltaTime * 1.5f;
  float x = cos(viewAngle);
  float y = sin(viewAngle);

  Vec3 direction = Vec3(x, 0, y);
  mCamera.SetViewTarget(mCamera.GetPosition() + direction);

  Vec3 playerView = mCamera.GetPosition() + throttle * direction * deltaTime * 2.0f;
  playerView.y = mCamera.GetPosition().y + m_InputFreeYMovement * deltaTime;
  mCamera.SetPosition(playerView);

  ImGui::Begin("Player");
  ImGui::Text("FPS: %f", shownFrameRate);
  ImGui::Text("Position: { %f, %f, %f }", playerView.x, playerView.y, playerView.z);
  ImGui::End();
  GameObject::Update(deltaTime);
}

void Player::Input(const InputCallbackData &inputData) {
  if (inputData.m_Name == HORIZONTAL_INPUT_NAME) {
    inputDirection = inputData.m_Value;
  } else if (inputData.m_Name == VERTICAL_INPUT_NAME) {
    throttle = inputData.m_Value;
  } else if (inputData.m_Name == DEBUG_VERTICAL_NAME) {
    m_InputFreeYMovement = inputData.m_Value;
  }
}
