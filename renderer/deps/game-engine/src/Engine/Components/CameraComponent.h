#ifndef CAMERA_COMPONENT_H
#define CAMERA_COMPONENT_H

#include "../Component.h"
#include "../CommonTypes.h"
#include <glm.hpp>

class CameraComponent : public Component {
public:
  CameraComponent();
  ~CameraComponent();

  void SetAsMainCamera();

  Vec3 GetPosition();
  void SetPosition(const Vec3 &position);
  Vec3 GetViewTarget();
  void SetViewTarget(const Vec3 &viewTarget);
  void SetFOV(const float fov);
  void SetScreenShader(const std::string &vertexShaderFile, const std::string &fragmentShaderFile);
  void SetUserData(const Mat4 &value);
  Mat4 mView;
  Mat4 mProjection;
private:
  bool mOrthographic;
  bool mAutomaticViewTarget;
  float mAspect;
  float mFov;
  Vec3 mPosition;
  Vec3 mViewTarget;

  void RecalulatePosition();
  
  const Vec3 AUTO_VIEW_TARGET = Vec3(0.0f, 0.0f, -1.0f);
};

#endif
