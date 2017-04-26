#version 330 core

in vec2 uv;

uniform sampler2D position_tex;
uniform sampler2D normal_tex;
uniform sampler2D ssao_blurred_tex;
uniform sampler2D tb_tex;

uniform vec3 camera_ws;
uniform vec3 light_ws;
uniform int draw_mode;

out vec4 FragmentColor;


const vec3 light_color = vec3(0.91, 0.97, 0.71);
const vec3 rgb_power = vec3(0.299f, 0.587f, 0.114f);

vec3 tex2d(vec2 uv)
{
    return texture(tb_tex, uv).rgb;
}

vec3 tex3d(in vec3 p, in vec3 n)
{
    p *= 1.4875;
    vec3 w = max(abs(n) - 0.317f, 0.0f);
    w /= dot(w, vec3(1.0f));
    mat3 rgb_samples = mat3(tex2d(p.yz), tex2d(p.zx), tex2d(p.xy));
    return sqrt(rgb_samples * w);
}

void main()
{             
    vec3 position_ws = texture(position_tex, uv).rgb;
    vec3 normal_ws = texture(normal_tex, uv).rgb;
    float ao = texture(ssao_blurred_tex, uv).r;

    vec3 diffuse_color = tex3d(position_ws, normal_ws);
    vec3 ambient_color = 0.37f * ao * diffuse_color;                                        // direct influence of AO on ambient color

    vec3 n = normalize(normal_ws);


    vec3 color = ambient_color;

    vec3 view = camera_ws - position_ws;
    vec3 light = light_ws - position_ws;

    float distance = length(light);
    vec3 v = normalize(view);
    vec3 l = light / distance;

    vec3 diffuse = max(dot(n, l), 0.0) * diffuse_color * light_color;
    vec3 h = normalize(l + v);  
    float specular_factor = pow(max(dot(n, h), 0.0), 18.0);

    vec3 specular = light_color * specular_factor;

    float attenuation = 1.0 / (1.0f + 0.009f * distance);
    color = color + sqrt(sqrt(ao)) * attenuation * (diffuse + specular);                          // moderate influence of AO on diffuse + specular colors

    if(draw_mode == 0)
        FragmentColor = vec4(color, 1.0);
    else if(draw_mode == 1)
        FragmentColor = vec4(0.15 * abs(position_ws), 1.0);
    else if(draw_mode == 2)
        FragmentColor = vec4(abs(normal_ws), 1.0);
    else if(draw_mode == 3)
        FragmentColor = vec4(vec3(ao), 1.0);
    else
        FragmentColor = vec4(diffuse_color, 1.0f);
}
