#version 330 core

uniform sampler2D scene_texture;

in vec2 uv;

out vec4 FragmentColor;

const float gamma = 0.6f;
const float numColors = 8.0f; 

void main()
{
	vec3 c = texture(scene_texture, uv).rgb;
	c = pow(c, vec3(gamma, gamma, gamma));
	c = c * numColors;
	c = floor(c);
	c = c / numColors;
	c = pow(c, vec3(1.0f / gamma));
	FragmentColor = vec4(c, 1.0f);
}

