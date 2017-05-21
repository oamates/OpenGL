#version 330 core

uniform sampler2D texture0;

layout (location = 0) out vec4 out_frag0;

in float Depth;

in vec2 uv;

void main(void)
{
	vec4 diffuseAndMask = texture(texture0, uv);
	if (diffuseAndMask.a < 0.5) discard;

	out_frag0 = vec4(diffuseAndMask.rgb, 1.0);
}