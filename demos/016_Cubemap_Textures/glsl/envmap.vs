#version 330 core

layout (location = 0) in vec4 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

out vec3 normal_ws;
out vec3 position_ws;
out vec3 view_direction;

uniform vec3 camera_ws;
uniform mat4 projection_view_matrix;
uniform mat4 view_matrix;


void main(void)
{
    normal_ws = normal;
    position_ws = vec3(position);
    view_direction = camera_ws - position_ws;
    gl_Position = projection_view_matrix * position;
}

