#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 position;
layout(location = 2) in vec2 texCoord;

layout(push_constant) uniform mdl {
  mat4 model;
};

out gl_PerVertex
{
  vec4 gl_Position;
};

layout (location = 1) out vec2 inFragTexCoords;

void main() {
  mat4 vk_model = model;
  vk_model[3].y *= -1;
  gl_Position = vk_model * vec4(position.x, -1 * position.y, position.z, 1.0f);
  inFragTexCoords = texCoord;
}
