#version 330 core

in vec2 uv;

uniform sampler2D diffuse_texture;

out vec4 FragmentColor;

void main()
{    
    FragmentColor = 0.25 * texture(diffuse_texture, uv);
}




