#version 330 core

in vec2 uv;

uniform sampler2D raymarch_tex;

out vec4 FragmentColor;

void main()
{
    FragmentColor = texture(raymarch_tex, uv);
}  