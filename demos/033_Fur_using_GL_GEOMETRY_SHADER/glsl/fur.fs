#version 430 core

layout (location = 0) out vec4 FragmentColor;

layout (binding = 0) uniform sampler2D fur_texture;
layout (binding = 1) uniform sampler2D diffuse_texture;

uniform vec4 fur_color = vec4(0.8, 0.8, 0.9, 1.0);

in GS_FS_VERTEX
{
    vec3 normal;
    vec2 tex_coord;
    flat float fur_strength;
} fragment_in;

void main()
{
    vec4 rgba = texture(fur_texture, fragment_in.tex_coord);
    vec4 diffuse_color = texture(diffuse_texture, fragment_in.tex_coord);
    float t = rgba.a;
    t *= fragment_in.fur_strength;

    vec3 color = mix(fur_color.xyz, diffuse_color.xyz, 0.7f);

    FragmentColor = fur_color * vec4(color, t);
}
