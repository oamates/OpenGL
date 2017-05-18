#version 430 core

const int LIGHT_COUNT = 4;

uniform sampler2D diffuse_tex;
uniform sampler2D normal_tex;
uniform vec3 camera_ws;
uniform vec3 light_ws[LIGHT_COUNT];

layout(binding = 10) uniform samplerCubeArrayShadow depth_tex;

in vec3 position_ws;
in vec3 normal_ws;
in vec3 tangent_x_ws;
in vec3 tangent_y_ws;
in vec2 uv;

out vec4 FragmentColor;


float depth_factor(vec3 l, int layer)
{
    vec3 a = abs(l);
    float z = max(a.x, max(a.y, a.z));

    float q = 1.0f - (1.0f / z);
    q = 0.5 + 0.5 * q;

    float shadow = texture(depth_tex, vec4(l, float(layer)), q);
    return shadow;
}

const vec3 light_color[4] = vec3[4]
(
    vec3(1.224, 0.254, 0.075),
    vec3(1.074, 0.987, 0.043),
    vec3(0.254, 1.311, 0.091),
    vec3(0.107, 0.196, 1.283)
);

float smooth_depth_factor(vec3 l, int layer)
{
    vec3 a = abs(l);
    float z = max(a.x, max(a.y, a.z));
    float q = 1.0f - (1.0f / z);
    q = 0.5 + 0.5 * q;
    const float ir2 = 0.70710678118;

    vec3 h = vec3(ir2, ir2, 0.0f);
    vec3 n = cross(l, h);
    vec3 b = cross(l, n);
    n = normalize(n);
    b = normalize(b);

    const vec2 w[8] = vec2[8]
    (
        vec2( 1.0,  0.0),
        vec2( ir2,  ir2),
        vec2( 0.0,  1.0),
        vec2(-ir2,  ir2),
        vec2(-1.0,  0.0),
        vec2(-ir2, -ir2),
        vec2( 0.0, -1.0),
        vec2( ir2, -ir2)
    );
    
    const float radius = 0.025f;
    float shadow = 0.0f;

    for(int i = 0; i < 8; ++i)
    {
        vec3 n0 = w[i].x * n + w[i].y * b;

        shadow += texture(depth_tex, vec4(l + radius * n0, float(layer)), q);
        shadow += texture(depth_tex, vec4(l + 0.5 * radius * n0, float(layer)), q);
    }
    return 0.0625f * shadow;
}

void main()
{
    vec3 nc = texture(normal_tex, uv).rgb - vec3(0.5f, 0.5f, 0.0f);
    vec3 n = normalize(nc.x * tangent_x_ws + nc.y * tangent_y_ws + nc.z * normal_ws);
    vec3 diffuse_color = texture(diffuse_tex, uv).rgb;
    vec3 view = camera_ws - position_ws;
    vec3 v = normalize(view);
    vec3 color = 0.145f * diffuse_color;

    for (int i = 0; i < LIGHT_COUNT; ++i)
    {
        vec3 diffuse = diffuse_color * light_color[i];
        vec3 specular = light_color[i];

        vec3 light = light_ws[i] - position_ws;
        float distance = length(light);
        vec3 l = light / distance;
        vec3 r = reflect(l, n);
        float dist_factor = 15.0 / (1.0f + distance);
    
        float cos_theta = clamp(dot(n, l), 0.0f, 1.0f);
        float cos_alpha = clamp(dot(v, r), 0.0f, 1.0f);                                                         

        float sf = smooth_depth_factor(-light, i);
        color += sf * dist_factor * cos_theta * (diffuse + specular * pow(cos_alpha, 40));
    }

    FragmentColor = vec4(color, 1.0f);
}
