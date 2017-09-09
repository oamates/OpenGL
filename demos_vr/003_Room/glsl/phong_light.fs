#version 330 core

uniform sampler2D diffuse_tex;
uniform sampler2D normal_tex;

in vec3 view;
in vec3 light;
in vec3 normal_ws;
in vec2 uv;

out vec4 FragmentColor;

void main()
{
    float light_distance = length(light);

    vec3 l = light / light_distance;
    vec3 n = normalize(normal_ws);
    vec3 e = normalize(view);
    vec3 r = reflect(l, n);

    float dp = dot(n, l);
    float cos_theta = clamp(dp, 0.0f, 1.0f);
    float cos_alpha = 0.0f;

    if (dp > 0.0f) cos_alpha = clamp(dot(e, r), 0.0f, 1.0f);

    vec4 material_diffuse_color = texture(diffuse_tex, uv);
    vec4 material_ambient_color = 0.1 * material_diffuse_color;
    vec4 material_specular_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);

    FragmentColor = material_ambient_color + 
                  + 10.0f * material_diffuse_color * cos_theta / (1.0 + light_distance)
                  + 15.0f * material_specular_color * pow(cos_alpha, 12) / (1.0 + light_distance);                 
}