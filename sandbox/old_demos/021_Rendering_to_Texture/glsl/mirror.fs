#version 330 core

in vec3 light_direction;
in vec3 view_direction;
in vec2 texture_coord;

uniform sampler2D diffuse_texture;
uniform sampler2D glass_texture;


out vec4 FragmentColor;

void main()
{
	vec4 glass_color = texture2D(glass_texture, texture_coord);
	vec4 reflected_color = texture2D(diffuse_texture, texture_coord);
	vec4 mixed_color = glass_color * reflected_color;

	float light_distance = length(light_direction);
	vec3 l = light_direction / light_distance; 										
	vec3 n = vec3(0.0f, 0.0f, 1.0f);
	vec3 e = normalize(view_direction);															
	vec3 r = reflect(l, n);	

	float dp = dot(n, l);
	float cos_theta = clamp(dp, 0.0f, 1.0f);
	float cos_alpha = 0.0f;
	if (dp > 0.0f) cos_alpha = clamp(dot(e, r), 0.0f, 1.0f);

	vec4 diffuse_color = 0.5 * glass_color + 1.5f * mixed_color + 0.8f * reflected_color;
	vec4 ambient_color = 0.3 * glass_color + 0.9f * mixed_color + 0.2f * reflected_color;
	vec4 specular_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);

	FragmentColor = ambient_color + 
					110.0f * diffuse_color * cos_theta / light_distance
                  + (70.0f * specular_color * pow(cos_alpha, 10) / light_distance);    

	//FragmentColor = 0.5 * glass_color + 1.5f * glass_color * reflected_color + 0.5f * reflected_color;
}





