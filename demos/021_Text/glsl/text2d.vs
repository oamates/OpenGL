#version 330 core

layout(location = 0) in vec2 position_in;
layout(location = 1) in vec2 uv_in;

uniform vec2 shift;
uniform vec2 scale;

out vec2 uv;

void main()
{
    gl_Position = vec4(shift + scale * position_in, 0.0f, 1.0f);
    uv = uv_in;
};

