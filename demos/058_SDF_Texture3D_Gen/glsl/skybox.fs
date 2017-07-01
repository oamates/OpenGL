#version 330 core

in vec3 uvw;
uniform samplerCube environment_tex;

layout (location = 0) out vec4 FragmentColor;

void main(void)
{
    FragmentColor = texture(environment_tex, uvw);
}