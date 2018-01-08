#version 330 core

in vec2 uv;

uniform sampler2D tex2d;

out vec4 FragmentColor;

void main()
{
    FragmentColor = texture(tex2d, uv);
}  