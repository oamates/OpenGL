#version 330 core

uniform vec3 light_color;

out vec4 FragmentColor;


void main()
{           
    FragmentColor = vec4(light_color, 1.0f);
}