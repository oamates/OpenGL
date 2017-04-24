#version 330 core

in vec3 color;

out vec4 FragmentColor;

void main()
{
    FragmentColor = vec4(color, 0.5f);
}