#version 330 core

uniform sampler2D texture0;
uniform vec2 LinMAD;
uniform int ResRatio;

layout (location = 0) out float out_frag0;

in vec2 TexCoord;

float ViewSpaceZFromDepth(float d)
{
	d = d * 2.0 - 1.0;								// [0,1] -> [-1,1] clip space
	return -1.0 / (LinMAD.x * d + LinMAD.y);		// Get view space Z
}

void main(void)
{
	float d = texelFetch(texture0, ivec2(gl_FragCoord.xy * ResRatio + vec2(0.0)), 0).r;
	//float d = texture(texture0, TexCoord + vec2(0.5) * vec2(1.0/1280.0, 1.0/720.0)).r;
	out_frag0 = ViewSpaceZFromDepth(d);
}