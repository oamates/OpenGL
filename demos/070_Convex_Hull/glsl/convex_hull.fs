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

    vec3 h = normalize(l + v);
    float cos_alpha = cos_theta * clamp(dot(n, h), 0.0f, 1.0f);

    vec3 diffuse_color = vec3(0.771f, 0.919f, 0.243f);
    vec3 ambient_color = 0.215f * diffuse_color;
    vec3 specular_color = vec3(1.0f);

    float diffuse_factor = 1.0f / (1.0 + distance);
    float specular_factor = 0.0625f * diffuse_factor;

    FragmentColor.rgb =  ambient_color + cos_theta * diffuse_factor * diffuse_color + cos_alpha * specular_factor * specular_color;
    FragmentColor.w = 1.0f;
}