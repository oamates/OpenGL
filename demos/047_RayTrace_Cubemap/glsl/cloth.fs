#version 330 core

in vec3 g_normal_ws;
in vec3 g_tangent_x_ws;
in vec3 g_tangent_y_ws;
in vec3 g_light;
in vec3 g_view;
in vec2 g_uv;

uniform sampler2D cloth_tex;
uniform sampler2D light_map;

out vec4 FragmentColor;

void main()
{
    vec3 light_color = texture(light_map, g_uv).rgb;
    vec3 n = normalize(g_normal_ws);
    vec3 l = normalize(g_light);
    vec3 r = reflect(-l, n);
    float specular = pow(clamp(dot(r, normalize(g_view)) + 0.1, 0.0, 1.0), 16.0);
    float diffuse = clamp(2.0 * (dot(n, l) - 0.5), 0.0, 1.0);
    float ambient = 0.275;
    vec3 color = texture(cloth_tex, g_uv).rgb;
    FragmentColor = vec4(color * ambient + light_color * color * diffuse + light_color * specular, 1.0);
}
