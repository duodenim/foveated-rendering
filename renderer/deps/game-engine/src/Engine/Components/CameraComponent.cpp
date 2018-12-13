#include "CameraComponent.h"
#include <gtc/matrix_transform.hpp>
#include "../Renderer/Frontend.h"
#include "../Config.h"

CameraComponent::CameraComponent() {
  mPosition = glm::vec3(0.0f, 0.02f, 1.0f);
  mViewTarget = glm::vec3(0.0f, 0.0f, 0.0f);
  mAutomaticViewTarget = false;
  float width = (float)Config::GetOptionInt("Width");
  float height = (float)Config::GetOptionInt("Height");
  mAspect = width / height;
  mFov = 45.5f;
  mView = glm::lookAt(mPosition, mViewTarget, glm::vec3(0.0f, 1.0f, 0.0f));
  mProjection = glm::perspective(mFov, mAspect, 0.1f, 100.0f);
}

CameraComponent::~CameraComponent() {

}

void CameraComponent::SetAsMainCamera() {
  RenderFrontend::SetMainCamera(this);
}

Vec3 CameraComponent::GetPosition() {
  return mPosition;
}

void CameraComponent::SetPosition(const Vec3 &position) {
  mPosition = position;
  RecalulatePosition();
}
Vec3 CameraComponent::GetViewTarget() {
  return mViewTarget;
}

void CameraComponent::SetViewTarget(const Vec3 &viewTarget) {
  mViewTarget = viewTarget;
  RecalulatePosition();
}

void CameraComponent::RecalulatePosition(){
  if (mAutomaticViewTarget) {
    mViewTarget = mPosition + AUTO_VIEW_TARGET;
  }
  mView = glm::lookAt(mPosition, mViewTarget, glm::vec3(0.0f, 1.0f, 0.0f));

  if (RenderFrontend::GetDepthMode() == DEPTH_MODE::ZERO_TO_ONE) {
    mProjection = glm::perspectiveZO(mFov, mAspect, 0.1f, 100.0f);
  } else {
    mProjection = glm::perspectiveNO(mFov, mAspect, 0.1f, 100.0f);
  }
}
void CameraComponent::SetFOV(const float fov) {
  mFov = fov;
  RecalulatePosition();
}

void CameraComponent::SetScreenShader(const std::string &vertexShaderFile, const std::string &fragmentShaderFile) {
  RenderFrontend::SetCameraShader(vertexShaderFile, fragmentShaderFile);
}

void CameraComponent::SetUserData(const Mat4 &value) {
  RenderFrontend::SetCameraUserData(value);
}

