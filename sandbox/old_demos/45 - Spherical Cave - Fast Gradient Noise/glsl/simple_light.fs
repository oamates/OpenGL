#version 430 core

layout(binding = 0) uniform sampler2D ssao_image;

in vec4 position_ws;
in vec4 normal_ws;
in vec4 view_direction;

uniform mat4 view_matrix;


out vec4 fragment_color;

const float two_pi = 6.283185307179586476925286766559f;
const float one_over_root3 = 0.57735026918962576450914878050196f;
const float cube_size = 7.0;

//const vec4 light_position = vec4(cube_size, cube_size, cube_size, 1.0f);

uniform vec4 light_position;
const vec4 light_color = vec4(1.5f, 1.5f, 0.2f, 1.0f);
const float light_intensity = 15000.0f;


vec3 smoother_step(vec3 t)
	{ return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f); };

float lerp(float a, float b, float alpha)
	{ return (1.0f - alpha) * a + alpha * b; };

vec3 gradient(vec3 p)
{
	return normalize(fract(sin(p) * 43758.5453));
};

float snoise(vec3 p)
{
	vec3 q = floor(p);
	p -= q;
	vec3 f = smoother_step(p);

	const vec3 d0 = vec3( 0.0f,  0.0f,  0.0f);
	const vec3 d1 = vec3( 0.0f, -1.0f,  0.0f);
	const vec3 d2 = vec3( 0.0f,  0.0f, -1.0f);	
	const vec3 d3 = vec3( 0.0f, -1.0f, -1.0f);  	
	const vec3 d4 = vec3(-1.0f,  0.0f,  0.0f);
	const vec3 d5 = vec3(-1.0f, -1.0f,  0.0f);
	const vec3 d6 = vec3(-1.0f,  0.0f, -1.0f);
	const vec3 d7 = vec3(-1.0f, -1.0f, -1.0f);

	return lerp(lerp(lerp(dot(gradient(q + d0), p + d0), dot(gradient(q + d4), p + d4), f.x), 
	                 lerp(dot(gradient(q + d1), p + d1), dot(gradient(q + d5), p + d5), f.x), f.y),
				lerp(lerp(dot(gradient(q + d2), p + d2), dot(gradient(q + d6), p + d6), f.x),
				     lerp(dot(gradient(q + d3), p + d3), dot(gradient(q + d7), p + d7), f.x), f.y), f.z);
};

vec3 marble_color(vec3 position)
{
	const vec3 Color1 = vec3(0.2f, 0.1f, 0.4f);
	const vec3 Color2 = vec3(0.7f, 0.6f, 0.8f);
	const float base_freq = 0.2f;

	vec4 noisevec = vec4(snoise(position * base_freq * 1.0f) * 8.0f,
						 snoise(position * base_freq * 2.0f) * 4.0f,
						 snoise(position * base_freq * 4.0f) * 2.0f,
						 snoise(position * base_freq * 8.0f) * 1.0f);
	noisevec = noisevec / 8.0f;

	float intensity = abs(noisevec[0] - 0.20f) + abs(noisevec[1] - 0.100f) + 
					  abs(noisevec[2] - 0.05f) + abs(noisevec[3] - 0.025f);

	intensity = clamp(intensity * 1.4f, 0.0f, 1.0f);
	
	return mix(Color1, Color2, intensity);
};


vec3 marble_color2(vec3 position)
{
	const vec3 Color1 = vec3(0.8f, 0.8f, 0.8f);
	const vec3 Color2 = vec3(0.2f, 0.2f, 0.5f);      
	const float base_freq = 0.2f;

	vec4 noisevec = vec4(snoise(position * base_freq * 1.0f) * 8.0f,
						 snoise(position * base_freq * 2.0f) * 4.0f,
						 snoise(position * base_freq * 4.0f) * 2.0f,
						 snoise(position * base_freq * 8.0f) * 1.0f);
	noisevec = noisevec / 8.0f;

	float intensity = abs(noisevec[0] - 0.200f) + abs(noisevec[1] - 0.100f) + 
					  abs(noisevec[2] - 0.050f) + abs(noisevec[3] - 0.025f);
	float sineval = sin(position.y * 12.0f + intensity * 8.0f) * 0.5f + 0.5f;
	
	return mix(Color1, Color2, sineval);
};


vec3 granite(vec3 position)
{
	const vec3 Color1 = vec3(0.35f, 0.3f, 0.2f);
	const vec3 Color2 = vec3(0.7f, 0.7f, 0.7f);

	const float base_freq = 0.2f;

	
	vec4 noisevec = vec4(snoise(position * base_freq * 1.0f) * 8.0f,
						 snoise(position * base_freq * 2.0f) * 4.0f,
						 snoise(position * base_freq * 4.0f) * 2.0f,
						 snoise(position * base_freq * 8.0f) * 1.0f);

	noisevec = noisevec / 8.0f;

	float intensity = min(1.0f, noisevec[3] * 12.0f);	
	return mix(Color1, Color2, intensity);
};

const vec2 screen_dim = vec2(1920.0f, 1080.0f);

void main()
{
	vec4 v = normalize(view_direction);


    float distance = length(light_position - position_ws);

    vec4 l = (light_position - position_ws) / distance; 
    float lambert_cosine = clamp(dot(normal_ws, l), 0.0f, 1.0f);

	vec4 r = reflect(l, normal_ws);
    float cos_alpha = clamp(dot(r, v), 0.0f, 1.0f) * lambert_cosine;
	

	float ambient_factor = 1.0f; //0.6f + 0.4f * texture(ssao_image, gl_FragCoord.xy / screen_dim).r;
//	float ambient_factor = texture(ssao_image, gl_FragCoord.xy / screen_dim).r;

//	vec4 marble_texture = vec4(vec3(abs(snoise(position_ws.xyz / 10.0f))), 1.0f);
	vec4 marble_texture = vec4(marble_color(position_ws.xyz), 1.0f);

	vec4 ambient_color = marble_texture;         

    vec4 material_diffuse_color = marble_texture;

    vec4 material_specular_color = normalize(ambient_color) * light_color;

    float diffuse_distance_factor = 10.0f / (distance * distance);
    float specular_distance_factor = 15.0f / (distance * distance);

	fragment_color = ambient_color +
                     + material_diffuse_color * light_intensity * light_color * lambert_cosine * diffuse_distance_factor
                     + material_specular_color * light_intensity * light_color * pow(cos_alpha, 20) * specular_distance_factor;

//	fragment_color = vec4(vec3(ambient_factor), 1.0f);

};



