#version 430 core

in vec3 position;
in vec3 normal;

const int ovrEye_Count = 2;
uniform vec3 camera_ws[ovrEye_Count];

uniform vec3 light_ws;
uniform sampler2D tb_tex;

out vec4 FragmentColor;


const vec3 rgb_power = vec3(0.299f, 0.587f, 0.114f);
const float bf = 0.297;

vec3 tex2d(vec2 uv)
{
    return texture(tb_tex, uv).rgb;
}

vec3 tex3d(vec3 p, vec3 n)
{
    p *= 1.4875;
    vec3 w = max(abs(n) - 0.317f, 0.0f);
    w /= dot(w, vec3(1.0f));
    mat3 rgb_samples = mat3(tex2d(p.yz), tex2d(p.zx), tex2d(p.xy));
    return sqrt(rgb_samples * w);
}

vec3 bump_normal(vec3 p, vec3 n)
{
    const vec2 e = vec2(0.0625, 0);
    mat3 mp = mat3(tex3d(p + e.xyy, n), tex3d(p + e.yxy, n), tex3d(p + e.yyx, n));
    mat3 mm = mat3(tex3d(p - e.xyy, n), tex3d(p - e.yxy, n), tex3d(p - e.yyx, n));
    vec3 g = (rgb_power * (mp - mm)) / e.x;
    return normalize(n - bf * g);
}

void main()
{
    vec3 v = normalize(camera_ws[gl_ViewportIndex] - position);
    vec3 l = normalize(light_ws  - position);

    vec3 diffuse_color = tex3d(position, normal);
    vec3 n = bump_normal(position, normal);

    float cos_theta = dot(n, l);

    vec3 color = vec3(0.0f);
    vec3 specular_color = vec3(0.707f);
    const float Ns = 80.0f;

    if (cos_theta > 0.0f) 
    {
        color += cos_theta * diffuse_color;
        vec3 h = normalize(l + v);
        float cos_alpha = max(dot(h, n), 0.0f);
        color += pow(cos_alpha, Ns) * specular_color;
    }

    FragmentColor = vec4(color, 1.0f);                 
}