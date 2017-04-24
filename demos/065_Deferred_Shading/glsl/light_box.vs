#version 330 core

layout (location = 0) in vec3 position_in;
layout (location = 1) in vec3 normal_in;
layout (location = 2) in vec2 uv_in;

uniform mat4 projection_view_matrix;
uniform mat4 model_matrix;

void main()
{
    gl_Position = projection_view_matrix * model_matrix * vec4(position_in, 1.0f);
}