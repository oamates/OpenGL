#version 330 core

in vec2 uv;

uniform sampler2D albedo_tex;
uniform sampler2D roughness_tex;

out vec3 color;

const float sqrt3p1_half = 1.36602540378;
const float minimal_albedo = 0.0625;

void main()
{
    vec3 roughness_average = textureLod(roughness_tex, uv, 5.0).rgb;
    float normalizing_factor = sqrt3p1_half / length(roughness_average);
    vec3 roughness = normalizing_factor * texture(roughness_tex, uv).rgb;
    vec3 albedo = vec3(minimal_albedo) + (1.0 - minimal_albedo) * texture(albedo_tex, uv).rgb;
    color = roughness * albedo;
}
