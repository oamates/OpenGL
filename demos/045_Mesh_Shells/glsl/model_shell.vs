#version 330 core

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;

out vec3 position;
out vec3 normal;

void main()
{
	normal = normal_in;
	position = position_in;
}