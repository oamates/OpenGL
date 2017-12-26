#version 330 core

layout(location = 0) in vec3 position_in;

uniform vec2 inv_scale;

out vec3 position_ws;

void main()
{
    position_ws = position_in;
    gl_Position = vec4(inv_scale * position_in.xy, 0.0f, 1.0f);
}
