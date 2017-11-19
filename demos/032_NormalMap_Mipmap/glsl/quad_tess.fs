#version 400 core

uniform sampler2D diffuse_tex;

in vec3 view;
in vec3 light;
in vec3 normal;
in vec2 uv;

out vec4 FragmentColor;

const float Ns = 20.0f;

void main()
{    
    vec3 l = normalize(light);
    vec3 n = normalize(normal);
    vec3 v = normalize(view);
    
    vec3 diffuse_color = texture(diffuse_tex, uv).rgb;
    vec3 ambient_color = 0.0625f * diffuse_color;
    vec3 specular_color = vec3(1.0f);
    vec3 color = ambient_color;

    float cos_theta = 0.5 + 0.5 * dot(n, l);
    color += cos_theta * diffuse_color;

    vec3 h = normalize(l + v);
    float cos_alpha = max(dot(h, n), 0.0f);
    color += 0.15 * pow(cos_alpha, Ns) * specular_color;

    FragmentColor = vec4(color, 1.0f);                 
}