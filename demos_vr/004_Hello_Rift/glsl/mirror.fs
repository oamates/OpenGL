#version 330 core

uniform sampler2D glass_texture;

in vec3 light_direction;
in vec3 view_direction;
in vec3 normal_ws;
in vec2 texture_coord;

out vec4 FragmentColor;

void main()
{
	vec4 glass_color = texture2D(glass_texture, texture_coord);

	float light_distance = length(light_direction);
	vec3 l = light_direction / light_distance; 										
	vec3 n = normal_ws;
	vec3 e = normalize(view_direction);															
	vec3 r = reflect(l, n);	

	float dp = dot(n, l);
	float cos_theta = clamp(dp, 0.0f, 1.0f);
	float cos_alpha = clamp(dot(e, r), 0.0f, 1.0f);

	vec4 diffuse_color = glass_color;
	vec4 ambient_color = 0.4 * glass_color;
	vec4 specular_color = vec4(1.0f, 0.8f, 0.4f, 1.0f);

	FragmentColor = ambient_color + 
					700.0f * diffuse_color * cos_theta / (light_distance * light_distance)
                  + 15000.0f * specular_color * pow(cos_alpha, 16) / (light_distance * light_distance);
    FragmentColor.w = 0.6;
}
