#version 330 core

uniform mat4 viewProjMatrix;
uniform mat4 modelMatrix;

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec4 vColor;

out vec4 vertColor;

void main()
{
    vertColor = vColor;
    gl_Position = viewProjMatrix * modelMatrix * vec4(vPosition, 1);
}
