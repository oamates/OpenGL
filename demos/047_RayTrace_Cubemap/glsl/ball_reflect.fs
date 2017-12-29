#version 330 core

in vec3 g_normal_ws;
in vec3 g_tangent_x_ws;
in vec3 g_tangent_y_ws;
in vec3 g_light;
in vec3 g_view;
in vec2 g_uv;

uniform sampler2DArray albedo_tex;
uniform samplerCube reflect_tex;
uniform float ball_idx;

out vec4 FragmentColor;

void main()
{
    vec3 uvw = vec3(g_uv, ball_idx);
    vec3 light_color = vec3(1.0);
    vec3 n = normalize(g_normal_ws);
    vec3 l = normalize(g_light);
    vec3 v = normalize(g_view);
    vec3 r = reflect(-l, n);
    vec3 vr = reflect(-v, n);
    vec3 refl_color = texture(reflect_tex, vr).rgb;
    float specular = pow(max(dot(r, v) + 0.1, 0.0), 64.0);
    float diffuse = max(dot(n, l) + 0.1, 0.0);

    const float reflectivity = 0.2;
    const float ambient = 0.2;

    vec3 color = texture(albedo_tex, uvw).rgb;
    FragmentColor = vec4(refl_color * reflectivity + color * ambient + light_color * color * diffuse + light_color * specular, 1.0);
}