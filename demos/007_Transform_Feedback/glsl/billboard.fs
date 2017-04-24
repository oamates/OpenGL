#version 330 core

uniform sampler2D particle_texture;

in vec2 uv;                                                                   
out vec4 FragmentColor;

void main()
{
    FragmentColor = texture2D(particle_texture, uv);
    if (FragmentColor.r >= 0.9 && FragmentColor.g >= 0.9 && FragmentColor.b >= 0.9) discard;
}