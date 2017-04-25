#version 330 core

layout (location = 0) out vec3 position;
layout (location = 1) out vec3 normal;

in vec3 position_ws;
in vec3 normal_ws;

void main()
{    
    position = position_ws;
    normal = normalize(normal_ws);
}