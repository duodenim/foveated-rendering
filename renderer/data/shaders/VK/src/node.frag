#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec4 outColor;

void main(){
  outColor = vec4(0.31, 0.41, 0.83, 1.0f);

}
