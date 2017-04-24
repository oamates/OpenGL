#version 330 core

uniform sampler2D diffuse_texture;
uniform sampler2D normal_texture; 

uniform vec3 light_worldspace;
uniform mat4 view_matrix;


in vec3 view_direction;
in vec3 light_direction;
in vec3 normal_direction;
in vec3 tangent_x_direction;
in vec3 tangent_y_direction;
in vec2 texture_coord;

out vec4 FragmentColor;

const vec3 light_color = vec3(1.0f, 1.0f, 0.2f);

void main()
{
	
	float light_distance = length(light_direction);
	vec3 l = light_direction / light_distance; 										

	vec3 components = texture(normal_texture, texture_coord).xyz - vec3(0.5f, 0.5f, 0.0f);

	vec3 n = normalize(components.x * tangent_x_direction + components.y * tangent_y_direction + components.z * normal_direction);
	vec3 e = normalize(view_direction);															
	vec3 r = reflect(l, n);
	
	float dp = dot(n, l);
	float cos_theta = clamp(dp, 0.0f, 1.0f);
	float cos_alpha = 0.0f;
	if (dp > 0.0f) cos_alpha = clamp(dot(e, r), 0.0f, 1.0f);


    vec3 material_diffuse_color = texture(diffuse_texture, texture_coord).xyz;
	vec3 material_ambient_color = 0.13f * mix(material_diffuse_color, light_color, 0.2f);
	vec3 material_specular_color = light_color;

	FragmentColor = vec4(material_ambient_color 
						 + 93.0f * material_diffuse_color * cos_theta / light_distance
	                     + 42.0f * material_specular_color * pow(cos_alpha, 6) / light_distance, 1.0f);
}





