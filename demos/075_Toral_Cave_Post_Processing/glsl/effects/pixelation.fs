#version 330 core

uniform sampler2D scene_texture;

in vec2 uv;

out vec4 FragmentColor;

const vec2 pixel_size = vec2(8.0f, 8.0f);
const vec2 screen_dim = vec2(1920.0f, 1080.0f);
const vec2 delta = pixel_size / screen_dim;

void main()
{
	vec2 coord = delta * floor(uv / delta);
	vec3 tc = texture(scene_texture, coord).rgb;
	FragmentColor = vec4(tc, 1.0);
}
