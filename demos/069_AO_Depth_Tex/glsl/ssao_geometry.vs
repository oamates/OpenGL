#version 330 core

layout (location = 0) in vec3 position_in;
layout (location = 1) in vec3 normal_in;

out vec3 position_ws;
out vec3 normal_ws;

uniform mat4 model_matrix;
uniform mat4 projection_view_matrix;
uniform mat3 normal_matrix;

void main()
{
    vec4 position_ws4 = model_matrix * vec4(position_in, 1.0f);
    normal_ws = normal_matrix * normal_in;
    gl_Position = projection_view_matrix * position_ws4;
    position_ws = position_ws4.xyz;
}