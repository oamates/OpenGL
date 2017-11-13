#version 330 core

in vec2 uv;

out vec4 FragmentColor;

uniform sampler2D teximage;
uniform int texlevel;
uniform int grayscale;
uniform float tex_value_inf;
uniform float tex_value_sup;

void main(void)
{
    vec3 c = (grayscale == 1) ? textureLod(teximage, uv, texlevel).rrr : 
                                textureLod(teximage, uv, texlevel).rgb;
    if (length(c.gb) == 0.0f) c.gb = c.rr;
    c = clamp((c - tex_value_inf) / (tex_value_sup - tex_value_inf), 0.0f, 1.0f);
    FragmentColor = vec4(c, 1.0f);
}

