#version 330 core

layout(location = 0) in vec2 uv_in;

uniform vec2 uv_max;
uniform vec2 uv_min;
uniform vec2 xy_max;
uniform vec2 xy_min;

out vec2 uv;

void main()
{
    uv = uv_min + uv_in * (uv_max - uv_min); 
    gl_Position = vec4(xy_min + uv_in * (xy_max - xy_min), 0.0f, 1.0f);
}


