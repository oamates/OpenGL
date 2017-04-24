#version 410 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent_x;
layout(location = 3) in vec3 tangent_y;
layout(location = 4) in vec2 uv;

out VS_GS_VERTEX
{
    vec3 position;
    vec3 normal;
    vec3 tangent_x;
    vec3 tangent_y;
    vec2 uv;
} vertex_out;

void main()
{
    vertex_out.position = position;
    vertex_out.normal = normal;
    vertex_out.tangent_x = tangent_x;
    vertex_out.tangent_y = tangent_y;
    vertex_out.uv = uv;
}