#version 330 core

in vec2 uv;

out vec4 FragmentColor;

uniform sampler2D teximage;
uniform int texlevel;
uniform int channel;
uniform float tex_value_inf;
uniform float tex_value_sup;

void main(void)
{
    vec3 c;

    if (channel == 0)
        c = textureLod(teximage, uv, texlevel).rrr;
    else if (channel == 1)
        c = textureLod(teximage, uv, texlevel).ggg;
    else if (channel == 2)
        c = textureLod(teximage, uv, texlevel).bbb;
    else if (channel == 3)
        c = textureLod(teximage, uv, texlevel).aaa;
    else
        c = textureLod(teximage, uv, texlevel).rgb;

    c = clamp((c - tex_value_inf) / (tex_value_sup - tex_value_inf), 0.0f, 1.0f);
    FragmentColor = vec4(c, 1.0f);
}

