#version 330 core

in vec2 uv;

out vec4 FragmentColor;

uniform sampler2D teximage;

void main(void)
{
    vec4 c = texture(teximage, uv);
    if (length(c.gb) == 0.0f) c.gb = c.rr;
    FragmentColor = c;
}

