#version 430 core


in vec3 normal_ws;
const float inv_sqrt3 = 0.5773502691896257645;
const vec3 light_direction = vec3(inv_sqrt3, inv_sqrt3, inv_sqrt3);

in vec2 texture_coords;
in float grey_scale;

uniform sampler2D diffuse_texture;

out vec4 FragmentColor;

void main()
{
	float cos_theta = dot(normal_ws, light_direction);
	float shadow_factor = 0.5 + 0.5 * cos_theta;
	FragmentColor = shadow_factor * (grey_scale * 0.77f) * texture(diffuse_texture, texture_coords);
}

