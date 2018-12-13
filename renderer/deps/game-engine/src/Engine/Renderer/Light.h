#pragma once
#include "../CommonTypes.h"
struct DirectionalLightData {
  Vec4 m_Direction;
  Vec4 m_AmbientColor;
  Vec4 m_DiffuseColor;
  Vec4 m_SpecularColor;
  Mat4 m_LightSpaceMatrix;
};

struct LightData {
  DirectionalLightData mDirectionalLight;
};