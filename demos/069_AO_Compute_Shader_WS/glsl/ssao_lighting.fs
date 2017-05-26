#version 330 core

in vec3 position_ws;
in vec3 normal_ws;

uniform sampler2D ssao_blurred_tex;
uniform sampler2D tb_tex;

uniform vec3 camera_ws;
uniform vec3 light_ws;
uniform float Ks;
uniform float Ns;
uniform float bf;
uniform float tex_scale;
uniform int draw_mode;

out vec4 FragmentColor;


const vec3 light_color = vec3(0.76, 1.08, 0.83);
const vec3 rgb_power = vec3(0.299f, 0.587f, 0.114f);

vec3 tex2d(vec2 uv)
{
    return texture(tb_tex, uv).rgb;
}

vec3 tex3d(in vec3 p, in vec3 n)
{
    p *= tex_scale;
    vec3 w = max(abs(n) - 0.317f, 0.0f);
    w /= dot(w, vec3(1.0f));
    mat3 rgb_samples = mat3(tex2d(p.yz), tex2d(p.zx), tex2d(p.xy));
    return pow(rgb_samples * w, vec3(1.16));
}

vec3 bumped_normal(in vec3 p, in vec3 n)
{
    const vec2 e = vec2(0.03125, 0);
    mat3 m = mat3(tex3d(p - e.xyy, n), tex3d(p - e.yxy, n), tex3d(p - e.yyx, n));
    vec3 g = rgb_power * m;                             // Converting to greyscale.
    g = (g - dot(tex3d(p , n), rgb_power)) / e.x;
    return normalize(n + g * bf);                                       // Bumped normal. "bf" - bump factor.
}

void main()
{             
    vec2 uv = gl_FragCoord.xy / textureSize(ssao_blurred_tex, 0);

    float ao = texture(ssao_blurred_tex, uv).r;

    vec3 diffuse_color = tex3d(position_ws, normal_ws);
    vec3 ambient_color = 0.25f * ao * diffuse_color;                                        // direct influence of AO on ambient color

    vec3 n = bumped_normal(position_ws, normalize(normal_ws));
    vec3 color = ambient_color;

    vec3 view = camera_ws - position_ws;
    vec3 light = light_ws - position_ws;

    float distance = length(light);
    vec3 v = normalize(view);
    vec3 l = light / distance;

    vec3 diffuse = max(dot(n, l), 0.0) * diffuse_color;
    vec3 h = normalize(l + v);  
    float specular_factor = Ks * pow(max(dot(n, h), 0.0), Ns);

    vec3 specular = light_color * specular_factor;

    float attenuation = 1.0 / (1.0 + 0.075 * distance);

    color = color + attenuation * ao * (diffuse + specular);                          // moderate influence of AO on diffuse + specular colors

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
