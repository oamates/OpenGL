#version 330 core

uniform sampler2D diffuse_tex;
uniform sampler2D normal_tex;

in vec3 view;
in vec3 light;
in vec3 tangent_x;
in vec3 tangent_y;
in vec3 normal;
in vec2 uv;

out vec4 FragmentColor;

void main()
{
    float light_distance = length(light);
    vec3 b = texture(normal_tex, uv).xyz - vec3(0.5f, 0.5f, 0.5f);
    vec3 n = normalize(b.x * tangent_x + b.y * tangent_y + b.z * normal);


    vec3 l = light / light_distance;
    vec3 e = normalize(view);
    vec3 r = reflect(l, n);

    float dp = dot(n, l);
    float cos_theta = clamp(dp, 0.0f, 1.0f);
    float cos_alpha = 0.0f;

    if (dp > 0.0f) cos_alpha = clamp(dot(e, r), 0.0f, 1.0f);

    vec3 diffuse_color = pow(texture(diffuse_tex, uv).rgb, vec3(2.2));
    vec3 ambient_color = 0.25 * diffuse_color;
    vec3 specular_color = vec3(1.0f);

    vec3 color =  ambient_color
                + 3.75f * diffuse_color * cos_theta / (1.0 + 1.65 * light_distance)
                + 1.50f * specular_color * pow(cos_alpha, 40) / (1.0 + 1.65 * light_distance);

    FragmentColor = vec4(color, 1.0f);
}