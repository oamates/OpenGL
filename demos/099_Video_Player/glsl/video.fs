#version 330 core

uniform sampler2D frameTex;

in vec2 uv;
out vec4 FragmentColor;

uniform float bump_intensity;

void main()
{
    const vec3 rgb_power = vec3(0.2126f, 0.7152f, 0.0722f);
    const vec3 rgb_rel_power = vec3(0.299f, 0.587f, 0.114f);
    vec3 diffuse_color = texture(frameTex, uv).rgb;

    // float L = bump_intensity * dot(rgb_power, diffuse_color);                                // luminosity variant 1
    float L = bump_intensity * sqrt(dot(rgb_rel_power * diffuse_color, diffuse_color));          // luminosity variant 2

    float Lx = dFdx(L);
    float Ly = dFdy(L);

    vec3 normal = vec3(-Lx, -Ly, 1.0f);
    vec3 n = normalize(normal);

    vec3 light = vec3(0.707, 0.0f, 0.707);
    float fade = max(dot(n, light), 0.0f);


    FragmentColor = vec4(fade * diffuse_color, 1.0f);
}