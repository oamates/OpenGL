#version 430 core

layout(binding = 0) uniform sampler2D diffuse_texture;
layout(binding = 1) uniform sampler2D normal_texture; 
layout(binding = 2) uniform samplerCube shadow_texture;
layout(binding = 3) uniform samplerCube depth_texture;

uniform mat4 view_matrix;

uniform vec4 light_ws;
in vec4 position_ws;
in vec3 view_direction;
in vec3 normal_direction;
in vec3 tangent_x_direction;
in vec3 tangent_y_direction;
in vec2 texture_coord;

out vec4 FragmentColor;

float smootherstep(float edge0, float edge1, float x)
{
    x = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return x * x * x * (x * (x * 6 - 15) + 10);
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
	vec4 material_ambient_color = 0.05 * material_diffuse_color;
	vec4 material_specular_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);


	float shadow_value = texture(shadow_texture, l).r;
    float shadow_factor = 0.3f + 0.7f * smootherstep(light_distance - 0.2, light_distance - 0.1, shadow_value);

	FragmentColor = material_ambient_color + 
					shadow_factor * (150.0f * material_diffuse_color * cos_theta / light_distance
                              + (40.0f * material_specular_color * pow(cos_alpha, 10) / light_distance));                 
};





