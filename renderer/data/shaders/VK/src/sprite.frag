#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout (location = 1) in vec2 inFragTexCoords;
layout (set = 1, binding = 0) uniform sampler2D diffuseTex;
layout (location = 0) out vec4 outColor;
void main() {
   outColor = texture(diffuseTex, vec2(inFragTexCoords.x, -1 * inFragTexCoords.y));
}
