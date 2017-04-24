#version 330 core

in vec3 normal_ws;
in vec3 position_ws;
in vec3 view_direction;

layout (location = 0) out vec4 FragmentColor;

uniform samplerCube environment_texture;

void main(void)
{
    vec3 texcoords = -reflect(view_direction, normalize(normal_ws));
    FragmentColor = vec4(0.3, 0.2, 0.1, 1.0) + vec4(0.97, 0.83, 0.79, 0.0) * texture(environment_texture, texcoords);
}
