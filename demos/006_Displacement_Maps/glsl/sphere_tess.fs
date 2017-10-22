#version 400 core

out vec3 position;
out vec3 normal;

uniform vec3 camera_ws;
uniform vec3 light_ws;

out vec4 FragmentColor;


void main()
{
    vec3 light = light_ws - position;
    vec3 view = camera_ws - position;

    vec3 l = normalize(light);
    vec3 v = normalize(view);
    vec3 n = normalize(normal);
    vec3 r = reflect(l, n);

    vec3 diffuse_color = vec3(0.77, 0.64, 0.45);
    vec3 ambient_color = 0.125f * diffuse_color;
    vec3 specular_color = ambient_color + vec3(0.125f);

    float cos_theta = dot(n, l);
    vec3 color = ambient_color;

    if (cos_theta > 0.0f) 
    {
        color += cos_theta * diffuse_color;
        vec3 h = normalize(l + v);
        float cos_alpha = max(dot(h, n), 0.0f);
        const float Ns = 80.0f;
        color += pow(cos_alpha, Ns) * specular_color;
    }

    FragmentColor = vec4(color, 1.0f);                 
}