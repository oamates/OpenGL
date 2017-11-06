#version 330 core

in vec2 uv;

out vec4 FragmentColor;

uniform sampler2D teximage;
uniform int texlevel;

void main(void)
{
    vec4 c = textureLod(teximage, uv, texlevel);
    if (length(c.gb) == 0.0f) c.gb = c.rr;
    FragmentColor = c;
}

