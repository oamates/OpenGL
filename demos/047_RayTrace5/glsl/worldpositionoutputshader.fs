#version 410 core

in vec3 position_ws;

out vec4 FragmentColor;

void main()
{
	FragmentColor = vec4(position_ws, 1.0f);
}