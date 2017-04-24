#version 330 core

in vec3 position_ws;
in vec3 normal_ws;
in vec2 uv;

uniform vec3 camera_ws;
uniform vec3 light_ws;

uniform sampler2D diffuse_texture;

out vec4 FragmentColor;

void main()
{
    vec3 v = normalize(camera_ws - position_ws);
    vec3 light = light_ws - position_ws;
    float dist = length(light);
    vec3 l = light / dist;
    vec3 n = normalize(normal_ws);

    vec3 diffuse_color = texture(diffuse_texture, uv).rgb;

    float cos_theta = dot(n, l);

    vec3 color = vec3(0.0f);
    vec3 specular_color = vec3(0.707f, 0.707f, 0.707f);
    const float Ns = 64.0f;

    if (cos_theta > 0.0f) 
    {
        color += cos_theta * diffuse_color / (1.0 + 0.21 * dist);

        // Phong lighting
//        vec3 r = reflect(-l, n);
//        float cos_alpha = max(dot(v, r), 0.0f);
//        float exponent = 0.25f * specular_exponent();
        
        // Blinn - Phong lighting
        vec3 h = normalize(l + v);
        float cos_alpha = max(dot(h, n), 0.0f);

        color += 0.87 * pow(cos_alpha, Ns) * specular_color / (1.0 + 0.21 * dist);
    }

    FragmentColor = vec4(color, 1.0f);                 
}
