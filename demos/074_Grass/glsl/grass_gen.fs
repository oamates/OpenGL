#version 330 core

uniform sampler2D blade_tex;

uniform vec3 camera_ws;
uniform vec3 light_ws;

in vec3 position;
in vec3 normal;
in vec2 uv;

out vec4 FragmentColor;

void main()
{
    vec3 l = normalize(light_ws - position);
    float q = 0.5f + 0.5f * abs(dot(normal, l));
    q *= (0.4f + 0.6f * uv.y);
    vec3 color = texture(blade_tex, uv).rgb;

    FragmentColor = vec4(q * color, 1.0f);
}


