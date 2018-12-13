#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 position;

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
  float gl_PointSize;
  float gl_ClipDistance[];
};

void main() {
  gl_Position = projection * view * model * vec4(position, 1.0f);
}
