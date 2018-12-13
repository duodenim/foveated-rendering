#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout (location = 1) in vec2 fragTexCoords;
layout (location = 0) out vec4 fragColor;
layout (set = 1, binding = 0) uniform sampler2D colorTexture;
void main() {
    fragColor = texture(colorTexture, fragTexCoords);
}