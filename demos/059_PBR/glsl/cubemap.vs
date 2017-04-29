#version 330 core

layout (location = 0) in vec3 position_in;

uniform mat4 projection;
uniform mat4 view;

out vec3 position_ws;

void main()
{
    position_ws = position_in;  

    gl_Position =  projection * view * vec4(position_in, 1.0);
}