#version 330 core

in vec3 position_ws;
in vec3 normal_ws;

uniform vec3 camera_ws;
uniform vec3 light_ws;

out vec4 FragmentColor;

const float light_intensity = 100.0f;

void main()
{
    vec3 n = normalize(normal_ws);

    vec3 view = camera_ws - position_ws;
	vec3 v = normalize(view);

    vec3 light = light_ws - position_ws;
    float distance = length(light);

    vec3 l = light / distance;
    float cos_theta = clamp(dot(n, l), 0.0f, 1.0f);

	vec3 r = reflect(-l, n);
    float cos_alpha = cos_theta * clamp(dot(r, v), 0.0f, 1.0f);

	vec3 ambient_color = vec3(0.271f, 0.309f, 0.043f);
    vec3 diffuse_color = ambient_color;
    vec3 specular_color = vec3(1.0f);

    float diffuse_distance_factor = 1.0f / distance;
    float specular_distance_factor = diffuse_distance_factor;

	FragmentColor.rgb =  ambient_color +
                         diffuse_color * light_intensity * cos_theta * diffuse_distance_factor;// +
                        //specular_color * light_intensity * pow(cos_alpha, 40.0f) * specular_distance_factor;
    FragmentColor.w = 1.0f;
}



