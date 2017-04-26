#version 330 core

uniform sampler2D ssao_blurred_tex;
uniform sampler2D tb_tex;

uniform vec3 camera_ws;
uniform vec3 light_ws;

in vec3 position_ws;
in vec3 normal_ws;
in float occlusion;

uniform mat4 view_matrix;
uniform int draw_mode;

out vec4 FragmentColor;

const vec3 light_color = vec3(1.0f);
const vec2 screen_dim = vec2(1920.0f, 1080.0f);

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
	vec3 view = camera_ws - position_ws;
	vec3 light = light_ws - position_ws;

    float distance = length(light);

    vec3 l = light / distance;
	vec3 v = normalize(view);
	vec3 n = normalize(normal_ws);

    vec2 uv = gl_FragCoord.xy / screen_dim;

    float fragment_ao = texture(ssao_blurred_tex, uv).r;
    float vertex_ao = occlusion;

    vec3 diffuse_color = tex3d(position_ws, normal_ws);
    vec3 ambient_color = 0.37f * fragment_ao * diffuse_color;

    vec3 color = ambient_color;
    vec3 diffuse = max(dot(n, l), 0.0) * diffuse_color * light_color;

    vec3 h = normalize(l + v);
    float specular_factor = pow(max(dot(n, h), 0.0), 18.0);
    vec3 specular = light_color * specular_factor;

    float attenuation = 1.0;
    color = color + sqrt(sqrt(fragment_ao)) * attenuation * (diffuse + specular);                          // moderate influence of AO on diffuse + specular colors

    if(draw_mode == 0)
        FragmentColor = vec4(color, 1.0);
    else if(draw_mode == 1)
        FragmentColor = vec4(0.15 * abs(position_ws), 1.0);
    else if(draw_mode == 2)
        FragmentColor = vec4(abs(normal_ws), 1.0);
    else if(draw_mode == 3)
        FragmentColor = vec4(vec3(fragment_ao), 1.0);
    else if(draw_mode == 4)
        FragmentColor = vec4(vec3(vertex_ao), 1.0);
    else
        FragmentColor = vec4(diffuse_color, 1.0f);
}