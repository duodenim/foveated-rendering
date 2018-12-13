#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout (location = 1) in vec2 inFragTexCoords;
layout (set = 1, binding = 0) uniform sampler2D diffuseTex;
layout (location = 0) out vec4 outColor;
void main() {
    if (gl_FrontFacing) {
        outColor = texture(diffuseTex, inFragTexCoords);
    } else {
        float alpha = texture(diffuseTex, inFragTexCoords).a;
        outColor = vec4(0.0f, 0.0f, 0.0f, alpha);
    }
}
