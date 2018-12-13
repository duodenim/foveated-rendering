#include "DirectionalLightComponent.h"
#include "../Renderer/Frontend.h"

DirectionalLightComponent::DirectionalLightComponent() {
  mCastsShadows = false;
  m_Direction = Vec4(0.0f, 0.0f, 0.0f, 0.0f);
  m_AmbientColor = Vec4(0.0f, 0.0f, 0.0f, 0.0f);
  m_SpecularColor = Vec4(0.0f, 0.0f, 0.0f, 0.0f);
  m_DiffuseColor = Vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

void DirectionalLightComponent::Update(const float deltaTime) {
  Component::Update(deltaTime);
  RenderFrontend::SetDirectionalLight(m_Direction, m_AmbientColor, m_DiffuseColor, m_SpecularColor);
}
void DirectionalLightComponent::SetDirection(const Vec3 direction) {
  m_Direction.x = direction.x;
  m_Direction.y = direction.y;
  m_Direction.z = direction.z;
}
void DirectionalLightComponent::SetAmbientColor(const Vec3 color) {
 m_AmbientColor.x = color.x;
 m_AmbientColor.y = color.y;
 m_AmbientColor.z = color.z;
}
void DirectionalLightComponent::SetSpecularColor(const Vec3 color) {
  m_SpecularColor.x = color.x;
  m_SpecularColor.y = color.y;
  m_SpecularColor.z = color.z;
}
void DirectionalLightComponent::SetDiffuseColor(const Vec3 color) {
  m_DiffuseColor.x = color.x;
  m_DiffuseColor.y = color.y;
  m_DiffuseColor.z = color.z;
}



