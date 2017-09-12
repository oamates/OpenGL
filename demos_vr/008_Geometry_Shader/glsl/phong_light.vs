#version 430 core

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;

uniform vec3 shift;

out vec3 position_ws;
out vec3 normal_ws;

void main()
{
    position_ws = shift + position_in;
    normal_ws = normal_in;
}