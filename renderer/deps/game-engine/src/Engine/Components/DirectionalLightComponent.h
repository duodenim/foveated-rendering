#pragma once

#include "../Component.h"
#include "../CommonTypes.h"
class DirectionalLightComponent : public Component {
public:
  DirectionalLightComponent();
  void Update(const float deltaTime);

  void SetDirection(const Vec3 direction);
  void SetAmbientColor(const Vec3 color);
  void SetSpecularColor(const Vec3 color);
  void SetDiffuseColor(const Vec3 color);
private:
  bool mCastsShadows;

  Vec4 m_Direction;
  Vec4 m_AmbientColor;
  Vec4 m_DiffuseColor;
  Vec4 m_SpecularColor;
};
