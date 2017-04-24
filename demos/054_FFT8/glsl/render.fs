#version 330 core

uniform float time;

const int n = 512;
const int m = 9;
const int mask = n - 1;
const vec2 center = vec2(0.5f * mask);

in vec3 position_ws;
in vec2 uv;

out vec4 FragmentColor;

void main()
{
    FragmentColor = vec4(0.5 + 0.5 * uv / center, 0.0, 1.0) * (0.9 - 0.4 * exp(-0.013 * abs(position_ws.z)));
}