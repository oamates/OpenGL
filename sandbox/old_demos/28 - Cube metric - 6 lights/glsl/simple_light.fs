#version 430 core

layout(binding = 0) uniform sampler2D diffuse_texture;
layout(binding = 1) uniform sampler2D normal_texture; 

layout(binding = 2) uniform samplerCube shadow_texture0;
layout(binding = 3) uniform samplerCube shadow_texture1;
layout(binding = 4) uniform samplerCube shadow_texture2;
layout(binding = 5) uniform samplerCube shadow_texture3;
layout(binding = 6) uniform samplerCube shadow_texture4;
layout(binding = 7) uniform samplerCube shadow_texture5;

uniform mat4 view_matrix;

in vec4 position_ws;
in vec3 view_direction;
in vec3 normal_direction;
in vec3 tangent_x_direction;
in vec3 tangent_y_direction;
in vec2 texture_coord;

out vec4 FragmentColor;

uniform vec4 light_ws[6];
uniform vec4 light_color[6];

const float light_power[6] = {50.0f, 50.0f, 50.0f, 50.0f, 50.0f, 50.0f};
const float specular_light_power[6] = {20.0f, 20.0f, 20.0f, 20.0f, 20.0f, 20.0f};


float smootherstep(float edge0, float edge1, float x)
{
    x = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return x * x * x * (x * (x * 6 - 15) + 10);
};

vec3 rand3dd (int n)
{
	return normalize(vec3(sin(224662.77 + 15.1 * n), cos(22.73921 + 912.3 * n), sin(1278.9353 + 5243.57 * n)));
};

void main()
{
	vec3 components = texture2D(normal_texture, texture_coord).xyz - vec3(0.5f, 0.5f, 0.5f);
	vec3 n = normalize(components.x * tangent_x_direction + components.y * tangent_y_direction + components.z * normal_direction);
	vec3 e = normalize(view_direction);															

	vec4 diffuse_color = texture2D(diffuse_texture, texture_coord);
	vec4 material_ambient_color = 0.25f * diffuse_color;
	vec4 specular_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);

    vec4 material_diffuse_color[6], 
		 material_specular_color[6];



    vec3 light_direction[6], l[6], r[6];
	float cos_theta[6], cos_alpha[6], light_distance[6];


	for (int i = 0; i < 6; ++i)
	{
	    light_direction[i] = vec3(light_ws[i] - position_ws);
		light_distance[i] = length(light_direction[i]);
		l[i] = light_direction[i] / light_distance[i];
		r[i] = reflect(l[i], n);
		cos_theta[i] = clamp(dot(n,l[i]), 0.0f, 1.0f);
		cos_alpha[i] = clamp(dot(e,r[i]), 0.0f, 1.0f);															

		material_diffuse_color[i] = mix(diffuse_color, light_color[i], 0.5);
		material_specular_color[i] = mix(specular_color, light_color[i], 0.5);

	};
	





	float shadow_factor[6] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

	
	for(int i = 0; i < 8; ++i)
	{
		vec3 q;
		float shadow_value, cube_dist;

		q = light_direction[0] + 0.03 * rand3dd(i);
		shadow_value = texture(shadow_texture0, q).r;
		cube_dist = max(max(abs(q.x), abs(q.y)), abs(q.z));
	    shadow_factor[0] += 0.1f + 0.9f * smootherstep(cube_dist - 0.1, cube_dist - 0.05, shadow_value);

		q = light_direction[1] + 0.03 * rand3dd(i);
		shadow_value = texture(shadow_texture1, q).r;
		cube_dist = max(max(abs(q.x), abs(q.y)), abs(q.z));
	    shadow_factor[1] += 0.1f + 0.9f * smootherstep(cube_dist - 0.1, cube_dist - 0.05, shadow_value);

		q = light_direction[2] + 0.03 * rand3dd(i);
		shadow_value = texture(shadow_texture2, q).r;
		cube_dist = max(max(abs(q.x), abs(q.y)), abs(q.z));
	    shadow_factor[2] += 0.1f + 0.9f * smootherstep(cube_dist - 0.1, cube_dist - 0.05, shadow_value);

	};

	shadow_factor[0] /= 8;
	shadow_factor[1] /= 8;
	shadow_factor[2] /= 8;
	shadow_factor[3] /= 8;
	shadow_factor[4] /= 8;
	shadow_factor[5] /= 8;

//	float shadow_value = texture(shadow_texture, l).r;
//	float cube_dist = max(max(abs(light_direction.x), abs(light_direction.y)), abs(light_direction.z));
//  float shadow_factor = 0.3f + 0.7f * smootherstep(cube_dist - 1.0, cube_dist - 0.5, shadow_value);

//	FragmentColor = material_ambient_color;
//	for (int i = 0; i < 3; ++i)
//	{
//		FragmentColor += shadow_factor[i] * (light_power[i] * material_diffuse_color[i] * cos_theta[i] / light_distance[i]
//                         + (specular_light_power[i] * material_specular_color[i] * pow(cos_alpha[i], 16) / light_distance[i]));
//	};
//	FragmentColor.w = 1.0f;






	FragmentColor = material_ambient_color;
	FragmentColor += shadow_factor[0] * (light_power[0] * material_diffuse_color[0] * cos_theta[0] / light_distance[0] + (specular_light_power[0] * material_specular_color[0] * pow(cos_alpha[0], 16) / light_distance[0]));
    FragmentColor += shadow_factor[1] * (light_power[1] * material_diffuse_color[1] * cos_theta[1] / light_distance[1] + (specular_light_power[1] * material_specular_color[1] * pow(cos_alpha[1], 16) / light_distance[1]));
	FragmentColor += shadow_factor[2] * (light_power[2] * material_diffuse_color[2] * cos_theta[2] / light_distance[2] + (specular_light_power[2] * material_specular_color[2] * pow(cos_alpha[2], 16) / light_distance[2]));
	FragmentColor += shadow_factor[3] * (light_power[3] * material_diffuse_color[3] * cos_theta[3] / light_distance[3] + (specular_light_power[3] * material_specular_color[3] * pow(cos_alpha[3], 16) / light_distance[3]));
	FragmentColor += shadow_factor[4] * (light_power[4] * material_diffuse_color[4] * cos_theta[4] / light_distance[4] + (specular_light_power[4] * material_specular_color[4] * pow(cos_alpha[4], 16) / light_distance[4]));
	FragmentColor += shadow_factor[5] * (light_power[5] * material_diffuse_color[5] * cos_theta[5] / light_distance[5] + (specular_light_power[5] * material_specular_color[5] * pow(cos_alpha[5], 16) / light_distance[5]));
	FragmentColor.w = 1.0f;

};





