#version 430 core

layout(binding = 1) uniform sampler2D diffuse_texture;

in VS_FS_VERTEX
{
    vec3 normal;
    vec2 texture_coord;
} vertex_in;

layout(location = 0) out vec4 FragmentColor;

const vec3 light_dir = vec3(0.707f, 0.707f, 0.0f);

void main()
{
    vec3 normal = vertex_in.normal;
    float fade_factor = clamp(0.5f + 0.5f * dot(normal, light_dir), 0.0f, 1.0f);
    FragmentColor = fade_factor * texture(diffuse_texture, vertex_in.texture_coord);
}
