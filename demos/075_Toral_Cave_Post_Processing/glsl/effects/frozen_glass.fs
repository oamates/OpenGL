#version 330 core                                                                        

uniform sampler2D scene_texture;

const vec2 pixel_size = vec2 (5.0f, 5.0f);
const float frequency = 0.115;
const vec2 screen_dim = vec2(1920.0f, 1080.0f);

in vec2 uv;
out vec4 FragmentColor;

vec4 spline(float x, vec4 c1, vec4 c2, vec4 c3, vec4 c4, vec4 c5, vec4 c6, vec4 c7, vec4 c8, vec4 c9)
{
	float w1 = 0.0f, w2 = 0.0f, w3 = 0.0f, w4 = 0.0f, w5 = 0.0f, w6 = 0.0f, w7 = 0.0f, w8 = 0.0f, w9 = 0.0f;
	float tmp = x * 8.0f;
	vec4 result = vec4(0.0f);
	if (tmp <= 1.0f)
	{
		w1 = 1.0f - tmp;
		w2 = tmp;
	}
	else if (tmp <= 2.0f)
	{
		tmp = tmp - 1.0f;
		w2 = 1.0f - tmp;
		w3 = tmp;
	}
	else if (tmp <= 3.0f)
	{
		tmp = tmp - 2.0f;
		w3 = 1.0f - tmp;
		w4 = tmp;
	}
	else if (tmp <= 4.0f)
	{
		tmp = tmp - 3.0;
		w4 = 1.0f - tmp;
		w5 = tmp;
	}
	else if (tmp <= 5.0f)
	{
		tmp = tmp - 4.0f;
		w5 = 1.0f - tmp;
		w6 = tmp;
	}
	else if (tmp <= 6.0f)
	{
		tmp = tmp - 5.0;
		w6 = 1.0 - tmp;
		w7 = tmp;
	}
	else if (tmp <= 7.0)
	{
		tmp = tmp - 6.0;
		w7 = 1.0 - tmp;
		w8 = tmp;
	}
	else
	{
		tmp = clamp(tmp - 7.0, 0.0, 1.0);
		w8 = 1.0 - tmp;
		w9 = tmp;
	};
	return w1*c1 + w2*c2 + w3*c3 + w4*c4 + w5*c5 + w6*c6 + w7*c7 + w8*c8 + w9*c9;
}

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


 
void main() 
{ 
	vec2 delta = pixel_size / screen_dim;
	vec2 ox = vec2(delta.x, 0.0f);
	vec2 oy = vec2(0.0f, delta.y);
	vec2 PP = uv - oy;
	vec4 C00 = texture(scene_texture, PP - ox);
	vec4 C01 = texture(scene_texture, PP);
	vec4 C02 = texture(scene_texture, PP + ox);
	PP = uv;
	vec4 C10 = texture(scene_texture, PP - ox);
	vec4 C11 = texture(scene_texture, PP);
	vec4 C12 = texture(scene_texture, PP + ox);
	PP = uv + oy;
	vec4 C20 = texture(scene_texture, PP - ox);
	vec4 C21 = texture(scene_texture, PP);
	vec4 C22 = texture(scene_texture, PP + ox);

	float n = snoise(100.0f * frequency * uv);
	n = mod(n, 0.111111f) / 0.111111f;

	FragmentColor = spline(n, C00, C01, C02, C10, C11, C12, C20, C21, C22);
	FragmentColor.w = 1.0f;
}