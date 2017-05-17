#version 430 core

uniform sampler2D diffuse_tex;
uniform sampler2D normal_tex;

layout(binding = 10) uniform samplerCubeShadow depth_tex;

uniform vec3 camera_ws;
uniform vec3 light_ws;

in vec3 position_ws;
in vec3 normal_ws;
in vec3 tangent_x_ws;
in vec3 tangent_y_ws;
in vec2 uv;

out vec4 FragmentColor;

float depth_factor(vec3 l)
{
    vec3 a = abs(l);
    float z = max(a.x, max(a.y, a.z));

    float q = 1.0f - (1.0f / z);

    float shadow = texture(depth_tex, vec4(l, q));
    return shadow;
}

void main()
{
    vec3 light = light_ws - position_ws;
    float distance = length(light);
    vec3 view = camera_ws - position_ws;

    vec3 l = light / distance;
    vec3 v = normalize(view);

    vec3 nc = texture(normal_tex, uv).rgb - vec3(0.5f, 0.5f, 0.0f);
    vec3 n = normalize(nc.x * tangent_x_ws + nc.y * tangent_y_ws + nc.z * normal_ws);

    vec3 r = reflect(l, n);
    
    float cos_theta = clamp(dot(n, l), 0.0f, 1.0f);
    float cos_alpha = clamp(dot(v, r), 0.0f, 1.0f);                                                         

    vec4 material_diffuse_color = texture(diffuse_tex, uv);
    vec4 material_ambient_color = 0.175f * material_diffuse_color;
    vec4 material_specular_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);

    vec2 dp = vec2(0.0125f, 0.0f);

    float a0 = depth_factor(-light);
    float shadow = a0;

    float dist_factor = 15.0 / (1.0f + distance);

    vec4 c = material_ambient_color + 
                    shadow * dist_factor * cos_theta * (material_diffuse_color + material_specular_color * pow(cos_alpha, 40));
    FragmentColor = c;
}
