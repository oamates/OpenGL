#version 330 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec3 position_cs;
in vec3 normal_cs;

void main()
{    
    gPosition = position_cs;
    gNormal = normalize(normal_cs);
    gAlbedoSpec.rgb = vec3(0.95);
}