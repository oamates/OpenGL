#version 430 core

layout(binding = 0) uniform sampler2D diffuse_texture;
layout(binding = 1) uniform sampler2D normal_texture; 
layout(binding = 2) uniform samplerCube shadow_texture;
layout(binding = 3) uniform samplerCube depth_texture;


uniform mat4 model_matrix[343];
uniform mat4 view_matrix;


uniform vec4 light_ws;
in vec4 position_ws;
in vec3 view_direction;
in vec3 normal_direction;
in vec3 tangent_x_direction;
in vec3 tangent_y_direction;
in vec2 texture_coord;

flat in int instance_id;


out vec4 FragmentColor;

//=========================================================================
// CLASSIC PERLIN NOISE
//=========================================================================

const float golden_ratio = 1.6180339887498948482045868343656f;
const vec4 icosahedron[] = 
{
	vec4 (0, -1, -golden_ratio, 0.0f),
	vec4 (0, -1,  golden_ratio, 0.0f),
	vec4 (0,  1, -golden_ratio, 0.0f),
	vec4 (0,  1,  golden_ratio, 0.0f),
	vec4 (-1, -golden_ratio, 0, 0.0f),
	vec4 (-1,  golden_ratio, 0, 0.0f),
	vec4 ( 1, -golden_ratio, 0, 0.0f),
	vec4 ( 1,  golden_ratio, 0, 0.0f),
	vec4 (-golden_ratio, 0, -1, 0.0f),
	vec4 ( golden_ratio, 0, -1, 0.0f),
	vec4 (-golden_ratio, 0,  1, 0.0f),
	vec4 ( golden_ratio, 0,  1, 0.0f),

	vec4(-1.0f, -1.0f, -1.0f, 0.0f),
	vec4( 1.0f, -1.0f, -1.0f, 0.0f),
	vec4(-1.0f,  1.0f, -1.0f, 0.0f),
	vec4( 1.0f,  1.0f, -1.0f, 0.0f),
	vec4(-1.0f, -1.0f,  1.0f, 0.0f),
	vec4( 1.0f, -1.0f,  1.0f, 0.0f),
	vec4(-1.0f,  1.0f,  1.0f, 0.0f),
	vec4( 1.0f,  1.0f,  1.0f, 0.0f),

	vec4 (-1/golden_ratio, 0, -golden_ratio, 0.0f),   
	vec4 ( 1/golden_ratio, 0, -golden_ratio, 0.0f),
	vec4 (-1/golden_ratio, 0,  golden_ratio, 0.0f),   						
	vec4 ( 1/golden_ratio, 0,  golden_ratio, 0.0f),
	vec4 (-golden_ratio, -1/golden_ratio, 0, 0.0f),   
	vec4 (-golden_ratio,  1/golden_ratio, 0, 0.0f),   
	vec4 ( golden_ratio, -1/golden_ratio, 0, 0.0f),   
	vec4 ( golden_ratio,  1/golden_ratio, 0, 0.0f),
	vec4 (0, -golden_ratio, -1/golden_ratio, 0.0f),   
	vec4 (0, -golden_ratio,  1/golden_ratio, 0.0f),
	vec4 (0,  golden_ratio, -1/golden_ratio, 0.0f),   	
	vec4 (0,  golden_ratio,  1/golden_ratio, 0.0f) 

};

const float scale_factor = 0.01f;


vec3 rand (int q)
{
	return vec3( sin(1123.275823 * q), sin(5671264.1235407 * q), sin(12342.1241549 * q) );
}

float smootherstep(float edge0, float edge1, float x)
{
    x = clamp((x - edge0)/(edge1 - edge0), 0.0, 1.0);
    return x*x*x*(x*(x*6 - 15) + 10);
}

void main()
{



    vec3 light_direction = vec3(light_ws - position_ws);

	
	float light_distance = length(light_direction);
	vec3 l = light_direction / light_distance; 										

	vec3 components = texture2D(normal_texture, texture_coord).xyz - vec3(0.5f, 0.5f, 0.5f);

	vec3 n = normalize(components.x * tangent_x_direction +
                       components.y * tangent_y_direction +
       				   components.z * normal_direction);

	vec3 e = normalize(view_direction);															
	vec3 r = reflect(l,n);
	
	float cos_theta = clamp(dot(n,l), 0.0f, 1.0f);
	float cos_alpha = clamp(dot(e,r), 0.0f, 1.0f);															

    vec4 material_diffuse_color = texture2D(diffuse_texture, texture_coord);
	vec4 material_ambient_color = 0.1 * material_diffuse_color;
	vec4 material_specular_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);


	float factor = 1.0f;
	float shadow_value = texture(shadow_texture, l).r;
	if (shadow_value < light_distance - 0.05) factor = 0.3f; 

    factor = 0.3f + 0.7f * smootherstep(light_distance - 0.05, light_distance, shadow_value);

//	FragmentColor = vec4(factor, factor, factor, 1.0f);





	FragmentColor = factor * (material_ambient_color + 
					100.0f * material_diffuse_color * cos_theta / light_distance
                + (40.0f * material_specular_color * pow(cos_alpha, 10) / light_distance));                 
};





