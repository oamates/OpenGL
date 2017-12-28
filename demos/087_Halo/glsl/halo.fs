#version 330 core

in float geomAlpha;
out vec4 FragmentColor;

void main()
{
    FragmentColor = vec4(0.5, 0.4, 0.3, pow(geomAlpha, 2.0));
}
