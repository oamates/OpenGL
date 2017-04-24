#version 330 core

in vec3 uvw;

uniform sampler3D volume_texture;
layout (location = 0) out vec4 FragmentColor;

void main(void)
{
    FragmentColor = vec4(0.7f, 0.1f, 1.5f, 1.0f) * texture(volume_texture, uvw).rrrr;
}
