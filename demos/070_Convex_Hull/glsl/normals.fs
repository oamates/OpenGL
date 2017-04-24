#version 330 core

in vec4 vertex_color;

out vec4 Fragment_color;

void main()
{
	Fragment_color = vertex_color;
}