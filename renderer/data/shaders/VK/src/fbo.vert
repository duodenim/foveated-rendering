#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 position;
layout(location = 2) in vec2 inTexCoords;
out gl_PerVertex
{
    vec4 gl_Position;
};
layout (location = 1) out vec2 fragTexCoords;
void main(){
    gl_Position = vec4(position.x, position.y, 1.0f, 1.0f);
    fragTexCoords = inTexCoords;
}
