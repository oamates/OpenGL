#version 430 core

uniform sampler2D bling; 
uniform sampler2D tex2;

in vec2 tex_coord; 

out vec4 FragmentColor;

void main()
{
    FragmentColor = texture(bling, tex_coord);
}
