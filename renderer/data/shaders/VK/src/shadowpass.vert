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
  gl_Position = model * vec4(position, 1.0);
  inFragTexCoords = texCoord;
}

