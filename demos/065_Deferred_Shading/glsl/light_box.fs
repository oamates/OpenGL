#version 330 core

uniform vec3 light_color;

layout (location = 0) out vec4 FragmentColor;


void main()
{           
    FragColor = vec4(lightColor, 1.0);
}