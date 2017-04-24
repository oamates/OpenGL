#version 330 core

in vec2 uv;

uniform vec4 text_color;
uniform sampler2D font_texture;

out vec4 FragmentColor;

void main()
{
    const float boundary_value = 0.5;
    const float threshold = 0.475;
    float alpha = smoothstep(boundary_value - threshold, boundary_value + threshold, texture(font_texture, uv).r);    
    FragmentColor = vec4(text_color.rgb, text_color.a * alpha);
};

