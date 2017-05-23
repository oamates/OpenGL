#version 330 core

layout (location = 0) out vec4 normal_cs_out;

in vec4 normal_cs;

void main()
{    
    normal_cs_out = normal_cs;
}