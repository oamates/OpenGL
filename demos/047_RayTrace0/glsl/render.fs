#version 330 core

in vec2 uv;

layout (location = 0) out vec4 FragmentColor;
uniform sampler2D raytrace_image;

void main()
{
    FragmentColor = texture(raytrace_image, uv);
}
