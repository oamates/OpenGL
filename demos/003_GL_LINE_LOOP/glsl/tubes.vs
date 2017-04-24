#version 400 core

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 tangent_in;
layout(location = 2) in vec3 normal_in;
layout(location = 3) in vec3 binormal_in;

out vec3 position_ms;
out vec3 tangent_ms;
out vec3 normal_ms;
out vec3 binormal_ms;

void main()
{
    position_ms = position_in;
    tangent_ms  = tangent_in; 
    normal_ms   = normal_in;  
    binormal_ms = binormal_in;
}


