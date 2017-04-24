#version 330 core

uniform sampler2D scene_texture;

const float luminance_threshold = 0.2f;
const float amplification_factor = 4.0f;
const vec3 vision_color = vec3(0.1f, 0.95f, 0.2f);
const vec3 luminance_color = vec3(0.30f, 0.59f, 0.11f);
const float noise_frequency = 50.0f;

in vec2 uv;

out vec4 FragmentColor;



float mod289(float x) 
	{ return x - floor(x * (1.0f / 289.0f)) * 289.0f; }
vec2 mod289(vec2 x) 
	{ return x - floor(x * (1.0f / 289.0f)) * 289.0f; }
vec3 mod289(vec3 x) 
	{ return x - floor(x * (1.0f / 289.0f)) * 289.0f; }
vec4 mod289(vec4 x) 
	{ return x - floor(x * (1.0f / 289.0f)) * 289.0f; }


float permute(float x) 
	{ return mod289(((x * 34.0f) + 1.0f) * x); }
vec2 permute(vec2 x) 
	{ return mod289(((x * 34.0f) + 1.0f) * x); }
vec3 permute(vec3 x) 
	{ return mod289(((x * 34.0f) + 1.0f) * x); }
vec4 permute(vec4 x) 
	{ return mod289(((x * 34.0f) + 1.0f) * x); }


float taylorInvSqrt(float r)
	{ return 1.79284291400159f - 0.85373472095314f * r; }
vec2 taylorInvSqrt(vec2 r)
	{ return 1.79284291400159f - 0.85373472095314f * r; }
vec3 taylorInvSqrt(vec3 r)
	{ return 1.79284291400159f - 0.85373472095314f * r; }
vec4 taylorInvSqrt(vec4 r)
	{ return 1.79284291400159f - 0.85373472095314f * r; }


float snoise(vec2 v)
{
	const vec4 C = vec4(0.211324865405187f, 0.366025403784439f, -0.577350269189626f, 0.024390243902439f);
	vec2 i  = floor(v + dot(v, C.yy));
	vec2 x0 = v - i + dot(i, C.xx);
	vec2 i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
	vec4 x12 = x0.xyxy + C.xxzz;
	x12.xy -= i1;
	i = mod289(i);
	vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 )) + i.x + vec3(0.0, i1.x, 1.0 ));
	vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
	m = m * m;
	m = m * m;
	vec3 x = 2.0 * fract(p * C.www) - 1.0;
	vec3 h = abs(x) - 0.5;
	vec3 ox = floor(x + 0.5);
	vec3 a0 = x - ox;
	m *= taylorInvSqrt(a0 * a0 + h * h);
	vec3 g = vec3(a0.x  * x0.x  + h.x  * x0.y, a0.yz * x12.xz + h.yz * x12.yw);
	return 130.0 * dot(m, g);
}



void main ()
{
	vec3 c0 = texture(scene_texture, uv).xyz;
	vec3 n = vec3(snoise(uv + 40.0f * c0.xy), 
				  snoise(uv + 50.0f * c0.xy),
				  snoise(uv + 60.0f * c0.xy));
	vec3 c = texture(scene_texture, uv + 0.005f * n.xy).rgb;
	if (dot(luminance_color, c) < luminance_threshold) c *= amplification_factor; 
	FragmentColor = vec4((c + (n * 0.2)) * vision_color, 1.0f);
}
