#version 330 core

uniform sampler2D scene_texture; 

const float time = 0; 

const vec2 screen_dim = vec2(1920.0f, 1080.0f);
const vec2 center = screen_dim / 2.0f;

uniform float radius = 540.0f;
uniform float angle = 0.8f;


in vec2 uv;
out vec4 FragmentColor;

void main()
{
	vec2 tc = uv * screen_dim - center;
	float dist = length(tc);
	if (dist < radius) 
	{
		float percent = (radius - dist) / radius;
		float theta = percent * percent * angle * 8.0f;
		float s = sin(theta);
		float c = cos(theta);
		tc = vec2(dot(tc, vec2(c, -s)), dot(tc, vec2(s, c)));
	}
	vec3 color = texture(scene_texture, (center + tc) / screen_dim).rgb;
	FragmentColor = vec4(color, 1.0);
}
