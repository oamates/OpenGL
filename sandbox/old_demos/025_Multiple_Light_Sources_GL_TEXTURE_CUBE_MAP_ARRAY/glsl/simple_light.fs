#version 430 core

//=======================================================================================================================================================================================================================
// texture samplers
//=======================================================================================================================================================================================================================

layout(binding = 0) uniform sampler2D diffuse_texture;
layout(binding = 1) uniform sampler2D normal_texture; 
layout(binding = 2) uniform samplerCubeArray shadow_texture;

//=======================================================================================================================================================================================================================
// fragment attributes
//=======================================================================================================================================================================================================================

in vec4 position_ws;
in vec3 view_direction;
in vec3 normal_direction;
in vec3 tangent_x_direction;
in vec3 tangent_y_direction;
in vec2 texture_coord;

out vec4 FragmentColor;


//=======================================================================================================================================================================================================================
// light variables
//=======================================================================================================================================================================================================================


const int MAX_LIGHT_SOURCES = 16;
uniform int LIGHT_SOURCES;

uniform vec4  light_ws[MAX_LIGHT_SOURCES];
uniform vec4  light_color[MAX_LIGHT_SOURCES];
uniform float light_power[MAX_LIGHT_SOURCES];
uniform float specular_light_power[MAX_LIGHT_SOURCES];

//=======================================================================================================================================================================================================================
// auxiliary functions
//=======================================================================================================================================================================================================================

float smootherstep(float edge0, float edge1, float x)
{
    x = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return x * x * x * (x * (x * 6 - 15) + 10);
};

vec3 rand3dd (int n)
{
	return normalize(vec3(sin(224662.77 + 15.1 * n), cos(22.73921 + 912.3 * n), sin(1278.9353 + 5243.57 * n)));
};

//=======================================================================================================================================================================================================================
// main function
//=======================================================================================================================================================================================================================

void main()
{
	vec3 bump = texture2D(normal_texture, texture_coord).xyz - vec3(0.5f, 0.5f, 0.5f);
	vec3 n = normalize(bump.x * tangent_x_direction + bump.y * tangent_y_direction + bump.z * normal_direction);
	vec3 e = normalize(view_direction);															

	vec4 diffuse_color = texture2D(diffuse_texture, texture_coord);
	vec4 specular_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);

	FragmentColor = 0.25f * diffuse_color;				// ambient component of the color

	for (int k = 0; k < LIGHT_SOURCES; ++k)
	{
	    vec3 light_direction = vec3(light_ws[k] - position_ws);
		float light_distance = length(light_direction);
		vec3 l = light_direction / light_distance;
		vec3 r = reflect(l, n);
		float cos_theta = clamp(dot(n,l), 0.0f, 1.0f);
		float cos_alpha = clamp(dot(e,r), 0.0f, 1.0f);															

		vec4 material_diffuse_color  = mix(diffuse_color,  light_color[k], 0.2f);
		vec4 material_specular_color = mix(specular_color, light_color[k], 0.2f);

		float shadow_factor = 0.0f;
		const float dispersion_radius = 0.03;
		for(int i = 0; i < 8; ++i)
		{
			vec3 q = light_direction + dispersion_radius * rand3dd(i);
			float shadow_value = texture(shadow_texture, vec4(q, k)).r;
			float cube_dist = max(max(abs(q.x), abs(q.y)), abs(q.z));
		    shadow_factor += 0.1f + 0.9f * smootherstep(cube_dist - 0.1f, cube_dist - 0.05f, shadow_value);

		};

		shadow_factor /= 8.0f;
		FragmentColor += shadow_factor * (light_power[k] * material_diffuse_color * cos_theta / light_distance + (specular_light_power[k] * material_specular_color * pow(cos_alpha, 16) / light_distance));
	};
	
	FragmentColor.w = 1.0f;								// avoid perspective lighting
};





