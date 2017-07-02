#version 330 core

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;

out vec3 position_ws;
out vec3 normal_ws;

void main()
{
    position_ws = scale * position_in;
    normal_ws = normal_in;
}