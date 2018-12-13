#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 position;
layout(location = 2) in vec2 texCoord;

layout(std140, binding = 0) uniform camera {
    mat4 view;
    mat4 projection;
};

layout(push_constant) uniform mdl {
  mat4 model;
};

layout (std140, binding = 7) uniform usr_data {
	mat4 usrData;
};

out gl_PerVertex
{
  vec4 gl_Position;
};

layout(location = 1) out vec2 fragTexCoords;

void main() {
  vec3 pos_shift = position;
  float x = position.x;
  float x_shift = 0.1 * sin((x * 3.14) + usrData[0][0]);
  pos_shift.z += x_shift;
  gl_Position = projection * view * model * vec4(pos_shift, 1.0f);
  fragTexCoords = texCoord;
}
