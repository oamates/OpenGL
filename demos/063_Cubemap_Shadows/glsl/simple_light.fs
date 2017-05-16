#version 430 core

uniform sampler2D diffuse_texture;
uniform sampler2D normal_texture;

layout(binding = 10) uniform samplerCubeShadow depth_texture;

struct motion3d_t
{
    vec4 shift;
    vec4 rotor;
};

layout (std430, binding = 0) buffer shader_data
{
    motion3d_t data[];
};

uniform mat4 view_matrix;
uniform vec4 light_ws;

out vec4 position_ms;
out vec4 position_ws;
out vec3 view_direction;
out vec3 normal_ws;
out vec3 tangent_x_ws;
out vec3 tangent_y_ws;
out vec2 texture_coord;

out vec4 FragmentColor;


vec2 fade(vec2 t) 
{
    return t*t*t*(t*(t*6.0 - 15.0) + 10.0);
};

vec4 permute(vec4 x)
{
	return mod(((x*34.0f) + 1.0f) * x, 289.0f);
};

float smootherstep(float edge0, float edge1, float x)
{
    x = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return x * x * x * (x * (x * 6.0f - 15.0f) + 10.0f);
}

float cnoise(vec2 P)
{
    vec4 Pi = floor(P.xyxy) + vec4(0.0, 0.0, 1.0, 1.0);
    vec4 Pf = fract(P.xyxy) - vec4(0.0, 0.0, 1.0, 1.0);
    Pi = mod(Pi, 289.0);
    vec4 ix = Pi.xzxz;
    vec4 iy = Pi.yyww;
    vec4 fx = Pf.xzxz;
    vec4 fy = Pf.yyww;
    vec4 i = permute(permute(ix) + iy);
    vec4 gx = 2.0 * fract(i * 0.0243902439) - 1.0;
    vec4 gy = abs(gx) - 0.5;
    vec4 tx = floor(gx + 0.5);
    gx = gx - tx;
    vec2 g00 = vec2(gx.x, gy.x);
    vec2 g10 = vec2(gx.y, gy.y);
    vec2 g01 = vec2(gx.z, gy.z);
    vec2 g11 = vec2(gx.w, gy.w);
    vec4 norm = 1.79284291400159 - 0.85373472095314 * vec4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11));
    g00 *= norm.x;
    g01 *= norm.y;
    g10 *= norm.z;
    g11 *= norm.w;
    float n00 = dot(g00, vec2(fx.x, fy.x));
    float n10 = dot(g10, vec2(fx.y, fy.y));
    float n01 = dot(g01, vec2(fx.z, fy.z));
    float n11 = dot(g11, vec2(fx.w, fy.w));
    vec2 fade_xy = fade(Pf.xy);
    vec2 n_x = mix(vec2(n00, n01), vec2(n10, n11), fade_xy.x);
    float n_xy = mix(n_x.x, n_x.y, fade_xy.y);
    return 2.3 * n_xy;
}



void main()
{
    vec3 light_direction = vec3(light_ws - position_ws);

	
	float light_distance = length(light_direction);
	vec3 l = light_direction / light_distance; 										

	vec3 components = texture2D(normal_texture, texture_coord).xyz - vec3(0.5f, 0.5f, 0.5f);
	vec3 n = normalize(components.x * tangent_x_ws + components.y * tangent_y_ws + components.z * normal_ws);

	vec3 e = normalize(view_direction);															
	vec3 r = reflect(l, n);
	
	float cos_theta = clamp(dot(n, l), 0.0f, 1.0f);
	float cos_alpha = clamp(dot(e, r), 0.0f, 1.0f);															

    vec4 material_diffuse_color = texture2D(diffuse_texture, texture_coord);
	vec4 material_ambient_color = 0.1 * material_diffuse_color;
	vec4 material_specular_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);

	float shadow_value = 0.0f;
	for (int i = -2; i <= 2; ++i)
		for (int j = -2; j <= 2; ++j)
			for (int k = -2; k <= 2; ++k) 
			{
				//shadow_value = shadow_value + texture(depth_texture, l).r;
			};
	shadow_value = shadow_value / 125.0f;

	float factor = 1.0f;
	if (light_distance > shadow_value + 0.5) factor = 0.4f;
	

	FragmentColor = factor * (material_ambient_color + 
					100.0f * material_diffuse_color * cos_theta / light_distance
                  + (40.0f * material_specular_color * pow(cos_alpha, 10) / light_distance));                 
    
}