#version 330 core

layout (location = 0) in vec3 position_in;

out vec3 position_ws;

void main()
{
    position_ws = position_in;
}