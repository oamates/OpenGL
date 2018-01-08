#version 330 core

in vec3 normal_ws;
in vec3 light;
in vec3 color;
in vec2 uv;

uniform sampler2D diffuse_tex;

out vec4 FragmentColor;

void main()
{
    vec3 n = normalize(normal_ws);
    vec3 l = normalize(light);
    vec3 c = texture(diffuse_tex, uv).rgb;
    float q = 0.3f + pow(max(dot(n, l) + 0.1f, 0.0) * 1.6f, 2.0f);

    vec3 color = q * (0.8f * c + 0.2f);
    FragmentColor = vec4(color, 1.0f);
}
