#version 430 core

in float intensity;

out vec4 FragmentColor;

void main(void)
{
    vec4 color = mix(vec4(0.0f, 0.2f, 1.0f, 1.0f), vec4(0.2f, 0.05f, 0.0f, 1.0f), intensity);
    FragmentColor = color;    
}
