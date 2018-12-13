#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec4 outColor;

layout (location = 0) in vec3 FragPos;
layout (location = 3) in vec3 FragNormal;
layout (location = 2) in vec3 FragColor;
layout (location = 4) in vec4 FragPosLightSpace;

const float bias = 0.001;

struct DirectionalLight {
  vec4 m_Direction;
  vec4 m_AmbientColor;
  vec4 m_DiffuseColor;
  vec4 m_SpecularColor;
  mat4 m_LightSpaceMatrix;
};

#define NUM_POINT_LIGHTS 18
struct PointLight {
  vec4 m_Position;
  vec4 m_DiffuseColor;
  vec4 m_SpecularColor;

  vec4 m_LightConstants; //x = constant, y = linear, z = quadratic
};

PointLight pLights[] = {
  { { -3.0, 4.0, 0.0, -3.0 }, { 1.0, 1.0, 1.0, 1.0 }, {1.0, 1.0, 1.0, 1.0}, {1.0, 0.22, 0.20, 0.0}},
  { { -3.0, 4.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0, 1.0 }, {1.0, 1.0, 1.0, 1.0}, {1.0, 0.22, 0.20, 0.0}},
  { { -3.0, 4.0, 0.0, 3.0 }, { 0.0, 1.0, 0.0, 1.0 }, {1.0, 1.0, 1.0, 1.0}, {1.0, 0.22, 0.20, 0.0}},
  { { 0.0, 4.0, 0.0, -3.0 }, { 0.0, 0.0, 1.0, 1.0 }, {1.0, 1.0, 1.0, 1.0}, {1.0, 0.22, 0.20, 0.0}},
  { { 0.0, 4.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0, 1.0 }, {1.0, 1.0, 1.0, 1.0}, {1.0, 0.22, 0.20, 0.0}},
  { { 0.0, 4.0, 0.0, 3.0 }, { 1.0, 0.0, 0.0, 1.0 }, {1.0, 1.0, 1.0, 1.0}, {1.0, 0.22, 0.20, 0.0}},
  { { 3.0, 4.0, 0.0, -3.0 }, { 0.0, 1.0, 0.0, 1.0 }, {1.0, 1.0, 1.0, 1.0}, {1.0, 0.22, 0.20, 0.0}},
  { { 3.0, 4.0, 0.0, 0.0 }, { 0.0, 0.0, 1.0, 1.0 }, {1.0, 1.0, 1.0, 1.0}, {1.0, 0.22, 0.20, 0.0}},
  { { 3.0, 4.0, 0.0, 3.0 }, { 1.0, 1.0, 1.0, 1.0 }, {1.0, 1.0, 1.0, 1.0}, {1.0, 0.22, 0.20, 0.0}},
  { { -3.0, 4.0, 0.0, -3.0 }, { 1.0, 1.0, 1.0, 1.0 }, {1.0, 1.0, 1.0, 1.0}, {1.0, 0.22, 0.20, 0.0}},
  { { -3.0, 4.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0, 1.0 }, {1.0, 1.0, 1.0, 1.0}, {1.0, 0.22, 0.20, 0.0}},
  { { -3.0, 4.0, 0.0, 3.0 }, { 0.0, 1.0, 0.0, 1.0 }, {1.0, 1.0, 1.0, 1.0}, {1.0, 0.22, 0.20, 0.0}},
  { { 0.0, 4.0, 0.0, -3.0 }, { 0.0, 0.0, 1.0, 1.0 }, {1.0, 1.0, 1.0, 1.0}, {1.0, 0.22, 0.20, 0.0}},
  { { 0.0, 4.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0, 1.0 }, {1.0, 1.0, 1.0, 1.0}, {1.0, 0.22, 0.20, 0.0}},
  { { 0.0, 4.0, 0.0, 3.0 }, { 1.0, 0.0, 0.0, 1.0 }, {1.0, 1.0, 1.0, 1.0}, {1.0, 0.22, 0.20, 0.0}},
  { { 3.0, 4.0, 0.0, -3.0 }, { 0.0, 1.0, 0.0, 1.0 }, {1.0, 1.0, 1.0, 1.0}, {1.0, 0.22, 0.20, 0.0}},
  { { 3.0, 4.0, 0.0, 0.0 }, { 0.0, 0.0, 1.0, 1.0 }, {1.0, 1.0, 1.0, 1.0}, {1.0, 0.22, 0.20, 0.0}},
  { { 3.0, 4.0, 0.0, 3.0 }, { 1.0, 1.0, 1.0, 1.0 }, {1.0, 1.0, 1.0, 1.0}, {1.0, 0.22, 0.20, 0.0}}
};


layout(std140, binding = 1) uniform lighting {
    DirectionalLight dLight;
};

layout (binding = 6) uniform sampler2D shadowMap;

vec3 GetPointLightColor(PointLight light, vec3 normalizedNormal) {
    vec3 unNormalizedDirection = light.m_Position.xyz - FragPos;
    float distance = length(unNormalizedDirection);
    vec3 lightDir = unNormalizedDirection / distance;
    float diffuseStrength = max(dot(normalizedNormal, lightDir), 0.0);
    float attenuation = 1.0 / (light.m_LightConstants.x + light.m_LightConstants.y * distance + light.m_LightConstants.z * distance * distance);

    return (attenuation * diffuseStrength * FragColor * light.m_DiffuseColor.xyz);
}


float CalcShadow() {
  vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
  projCoords.xy = projCoords.xy * 0.5 + 0.5;

  float closestDepth = texture(shadowMap, projCoords.xy).r;
  float currentDepth = projCoords.z;

  return (currentDepth - bias) > closestDepth ? 1.0 : 0.0;
}
void main(){
  vec3 norm = normalize(FragNormal);
  vec3 lightDir = normalize(-dLight.m_Direction.xyz);
  float strength = max(dot(norm, lightDir), 0.0);
  vec3 color = (1.0 - CalcShadow()) * dLight.m_DiffuseColor.xyz * strength * FragColor;
  color += dLight.m_AmbientColor.xyz;

  for (int i = 0; i < NUM_POINT_LIGHTS; i++) {
    color += GetPointLightColor(pLights[i], norm);
  }
  outColor =  vec4(color, 1.0f);
}