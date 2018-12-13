#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

layout(std140, binding = 0) uniform camera {
    mat4 view;
    mat4 projection;
};

layout(push_constant) uniform mdl {
  mat4 model;
};

out gl_PerVertex
{
  vec4 gl_Position;
};

layout (location = 3)out vec3 FragNormal;
layout (location = 1) out vec2 inFragTexCoords;
void main() {
  gl_Position = projection * view * model * vec4(position, 1.0);
  FragNormal = mat3(transpose(inverse(model))) * normal;
  inFragTexCoords = texCoord;
}