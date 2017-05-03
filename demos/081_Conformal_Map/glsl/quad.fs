#version 330 core

in vec2 uv;

uniform sampler2D conformal_tex;
uniform sampler2D input_tex;

out vec4 FragmentColor;

void main()
{
    FragmentColor = texture(conformal_tex, uv);
}  