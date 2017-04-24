#version 430 core

layout (location = 0) in vec4 position_in;
layout (location = 1) in vec3 normal_in;
layout (location = 2) in vec2 uv_in;

uniform mat4 view_matrix;
uniform mat4 projection_matrix;

out VS_FS_VERTEX
{
    vec3 normal;
    vec2 texture_coord;
} vertex_out;

void main(void)
{
    vertex_out.normal = normal_in;
    vertex_out.texture_coord = uv_in;
    gl_Position = projection_matrix * view_matrix * position_in;
}
