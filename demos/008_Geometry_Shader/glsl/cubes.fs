#version 330 core

in vec4 color;
in vec4 normal;
in vec4 tangent_x;
in vec4 tangent_y;
in vec4 view_direction;
in vec2 texture_coord;


uniform vec4 light_direction;
const vec4 light_color = vec4(1.0, 1.0, 1.0, 0.0);
const vec4 specular_color = vec4(1.0, 1.0, 1.0, 0.0);

uniform sampler2D texture_sampler;



out vec4 FragmentColor;

void main()
{
    vec3 q = vec3(texture2D(texture_sampler, texture_coord)) - vec3(0.5, 0.5, 0.5);
    vec4 l = normalize(light_direction);
	vec4 n = normalize(q.x * tangent_x + q.y * tangent_y + q.z * normal);																	
	vec4 e = normalize(view_direction);															
	vec4 r = reflect(-l,n);

	float cos_theta = clamp(dot(n,l), 0.0f, 1.0f);
	float cos_alpha = clamp(dot(e,r), 0.0f, 1.0f);															

    FragmentColor = color * light_color * cos_theta + color * specular_color * pow(cos_alpha, 5);
}






