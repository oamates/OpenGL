#version 430 core

in vec3 position;
in vec3 normal;

uniform vec3 light_ws;
uniform sampler2D tb_tex;

out vec4 FragmentColor;


const vec3 rgb_power = vec3(0.299f, 0.587f, 0.114f);
const float bf = 0.597;

vec3 tex2d(vec2 uv)
{
    return texture(tb_tex, uv).rgb;
}

vec3 tex3d(vec3 p, vec3 n)
{
    vec3 w = max(abs(n) - 0.317f, 0.0f);
    w /= dot(w, vec3(1.0f));
    mat3 rgb_samples = mat3(tex2d(p.yz), tex2d(p.zx), tex2d(p.xy));
    return sqrt(rgb_samples * w);
}

vec3 bump_normal(vec3 p, vec3 n)
{
    const vec2 e = vec2(0.03125, 0);
    mat3 mp = mat3(tex3d(p + e.xyy, n), tex3d(p + e.yxy, n), tex3d(p + e.yyx, n));
    mat3 mm = mat3(tex3d(p - e.xyy, n), tex3d(p - e.yxy, n), tex3d(p - e.yyx, n));
    vec3 g = (rgb_power * (mp - mm)) / e.x;
    return normalize(n - bf * g);
}

const float ambient_factor = 0.25;

void main()
{    
    vec3 l = normalize(light_ws  - position);

    vec3 diffuse_color = tex3d(position, normal);
    vec3 n = bump_normal(position, normal);

    float cos_theta = 0.5 + 0.5 * dot(n, l);

    vec3 ambient_color = ambient_factor * cos_theta * diffuse_color;

    FragmentColor = vec4(ambient_color, 1.0f);
}