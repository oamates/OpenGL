#version 330 core

uniform float time;

in vec3 position_ws;
in vec2 uv;

out vec4 FragmentColor;

void main()
{
    FragmentColor = vec4(uv, 0.0, 1.0) * (1.0 - 0.4 * exp(-0.16 * abs(position_ws.z)));
}