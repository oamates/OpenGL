#version 330 core

uniform sampler2D blade_tex;

uniform vec3 camera_ws;
uniform vec3 light_ws;

in vec3 position;
in vec3 normal;

out vec4 FragmentColor;

vec3 tex3D(vec3 p, vec3 n)
{
    p += 10.0 * n;
    n = max(abs(n) - 0.35f, 0.0f);
    n /= dot(n, vec3(1.0f));
    vec3 tx = texture(blade_tex, p.zy).xyz;
    vec3 ty = texture(blade_tex, p.xz).xyz;
    vec3 tz = texture(blade_tex, p.xy).xyz;
    vec3 c = tx * tx * n.x + ty * ty * n.y + tz * tz * n.z;
    return pow(c, vec3(0.56));
}

void main()
{
    vec3 l = normalize(light_ws - position);
    float q = 0.5f + 0.5f * abs(dot(normal, l));
    vec3 color = tex3D(position, normal).rgb;
    FragmentColor = vec4(q * color, 1.0f);
}


