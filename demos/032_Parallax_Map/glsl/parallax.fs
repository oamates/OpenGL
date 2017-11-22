#version 330 core

in vec3 light_ts;
in vec3 view_ts;
in vec2 uv;

out vec4 FragmentColor;

uniform sampler2D diffuse_tex;
uniform sampler2D normal_tex;
uniform sampler2D height_tex;

uniform float height_scale;
uniform int parallax;
uniform int division;

void main()
{
    vec3 v = normalize(view_ts);
    vec2 q = uv;

    if (q.x > 1.0 || q.y > 1.0 || q.x < 0.0 || q.y < 0.0) discard;

    if (parallax != 0)
    {
        vec3 height = texture(height_tex, q).rgb;
        float z = height.z;

        vec2 view_xy = normalize(v.xy);
        if (division != 0)
            view_xy /= v.z;
        q += z * height_scale * view_xy;
    }

    vec3 n = 2.0 * texture(normal_tex, q).rgb - 1.0;
    n = normalize(n);

    vec3 base_color = texture(diffuse_tex, q).rgb;
    vec3 ambient = 0.1 * base_color;

    vec3 l = normalize(light_ts);

    float cos_theta = max(dot(l, n), 0.0);
    vec3 diffuse = cos_theta * base_color;

    vec3 r = reflect(-l, n);
    vec3 h = normalize(l + v);

    float cos_alpha = 0.2 * pow(max(dot(n, h), 0.0), 32.0);
    vec3 specular = vec3(cos_alpha);

    FragmentColor = vec4(ambient + diffuse + specular, 1.0);
}
