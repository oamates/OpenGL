#version 330 core

in vec3 color;
out vec4 Fragment_color;

void main()
{
	Fragment_color = vec4(color, 1.0f);
}