#version 450 core
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 3) in vec3 color;

struct DirectionalLight {
  vec4 m_Direction;
  vec4 m_AmbientColor;
  vec4 m_DiffuseColor;
  vec4 m_SpecularColor;
  mat4 m_LightSpaceMatrix;
};

layout(std140, binding = 0) uniform camera {
    mat4 view;
    mat4 projection;
};

layout(std140, binding = 1) uniform lighting {
    DirectionalLight dLight;
};

layout(push_constant) uniform mdl {
  mat4 model;
};

out gl_PerVertex
{
  vec4 gl_Position;
};

layout (location = 0)out vec3 FragPos;
layout (location = 3)out vec3 FragNormal;
layout (location = 2) out vec3 FragColor;
layout (location = 4) out vec4 FragPosLightSpace;

void main() {
  gl_Position = projection * view * model * vec4(position, 1.0);
  FragPos = vec3(model * vec4(position, 1.0));
  FragNormal = mat3(transpose(inverse(model))) * normal;
  FragColor = color;
  FragPosLightSpace = dLight.m_LightSpaceMatrix * vec4(FragPos, 1.0);
}

