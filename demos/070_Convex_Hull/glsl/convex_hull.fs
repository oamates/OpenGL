#version 330 core

in vec4 position_ws;
in vec4 normal_ws;

uniform mat4 view_matrix;

out vec4 fragment_color;

uniform vec4 light_position;

const vec4 light_color = vec4(1.6f, 0.1f, 0.1f, 1.0f);
const float light_intensity = 1000.0f;

void main()
{
    vec4 view_direction = view_matrix[3] - position_ws;

	vec4 v = normalize(view_direction);
    float distance = length(light_position - position_ws);

    vec4 l = (light_position - position_ws) / distance; 
    float lambert_cosine = clamp(dot(normal_ws, l), 0.0f, 1.0f);

	vec4 r = reflect(l, normal_ws);
    float cos_alpha = clamp(dot(r, v), 0.0f, 1.0f);
	
	vec4 ambient_color = vec4(0.05f, 0.30f, 0.02f, 0.0f);

    vec4 material_diffuse_color = ambient_color;
    float diffuse_distance_factor = 1.0f / distance;

	fragment_color = ambient_color + material_diffuse_color * light_intensity * light_color * lambert_cosine * diffuse_distance_factor;
}



