#version 330 core

in vec2 uv;

out vec4 FragmentColor;

uniform sampler2D teximage;

void main(void)
{
    FragmentColor = texture(teximage, uv);
}

