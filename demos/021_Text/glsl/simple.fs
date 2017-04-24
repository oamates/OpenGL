#version 420 core

layout(binding = 0) uniform sampler2D diffuse_texture;

const vec3 light_dir = vec3(0.12, 0.54252, 0.835);

in vec3 normal_ws;
in vec2 uv;



out vec4 FragmentColor;


void main()
{
    float lambert_factor = clamp(0.3f + 0.7f * dot(light_dir, normal_ws), 0.0f, 1.0f);

    vec4 color = texture(diffuse_texture, uv);
    color.w = 0.33;

    FragmentColor = lambert_factor * color;

}
