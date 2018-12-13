#version 450 core
#extension GL_ARB_separate_shader_objects : enable
layout (set = 1, binding = 0) uniform sampler2D diffuseTex;
layout (location = 1) in vec2 inFragTexCoords;
void main() {
    if (texture(diffuseTex, inFragTexCoords).a < 0.1) {
        discard;
    }
}
