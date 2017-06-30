#version 330 core

layout (location = 0) in vec3 position_in;
layout (location = 1) in vec3 normal_in;
layout (location = 2) in vec2 uv_in;

uniform mat4 projection_view_matrix;
uniform vec3 camera_ws;

out vec3 position_ws;
out vec3 normal_ws;
out vec3 view;


void main(void)
{
    normal_ws = normal_in;
    position_ws = position_in;
    view = camera_ws - position_ws;
    gl_Position = projection_view_matrix * vec4(position_ws, 1.0f);
}

