#version 430 core

layout(binding = 0) uniform sampler2D ssao_image;

const vec2 screen_dim = vec2(1920.0f, 1080.0f);

out vec4 fragment_color;

void main()
{
	float ambient_factor = texture(ssao_image, gl_FragCoord.xy / screen_dim).r;
	fragment_color = vec4(vec3(0.2f + 0.8f * ambient_factor), 1.0f);
};



