#version 330 core

uniform sampler2D scene_texture;

in vec2 uv;

out vec4 FragmentColor;

const vec3 colors[3] = vec3[3]
(
	vec3(0.0f, 0.0f, 1.0f),
	vec3(1.0f, 1.0f, 0.0f),
	vec3(1.0f, 0.0f, 0.0f)
);

void main()
{
	vec3 pixcol = texture(scene_texture, uv).rgb;
	float lum = (pixcol.r + pixcol.g + pixcol.b) / 3.0f;
	int ix = (lum < 0.5) ? 0 : 1;
	vec3 tc = mix(colors[ix], colors[ix + 1], (lum - float(ix) * 0.5f) / 0.5f);
	FragmentColor = vec4(tc, 1.0);
}
