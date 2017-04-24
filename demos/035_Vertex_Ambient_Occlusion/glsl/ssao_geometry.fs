#version 330 core

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;

in vec3 position_cs;
in vec3 normal_cs;
in float vertex_occlusion;
in float hue;


void main()
{    
    gPosition = vec4(position_cs, vertex_occlusion);
    gNormal = vec4(normalize(normal_cs), hue);
}