#version 330 core

in vec3 position_ws;
in vec3 normal_ws;
in vec2 uv;

uniform vec3 color;
uniform vec3 camera_ws;

out vec4 FragmentColor;


const float pi = 3.14159265359f;

void main()
{
    vec3 white = vec3(1.0);
    vec3 v = normalize(camera_ws - position_ws);
    vec3 n = normalize(normal_ws);
    float q = 0.5 + 0.5 * dot(v, n);

    vec3 c = mix(white, color, pow(q, 2.4));

    FragmentColor = vec4(c, 1.0f);
}
