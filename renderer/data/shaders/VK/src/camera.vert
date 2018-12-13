#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 position;
layout(location = 2) in vec2 inTexCoords;
out gl_PerVertex
{
    vec4 gl_Position;
};
layout (location = 1) out vec2 fragTexCoords;

layout (std140, binding = 7) uniform usr_data {
	mat4 usrData;
};

void main(){
    float shift = usrData[0][1] * sin((position.x * 3.14) + usrData[0][0]);
    gl_Position = vec4(position.x, position.y + shift, 0.99f, 1.0f);
    fragTexCoords = inTexCoords;
}
