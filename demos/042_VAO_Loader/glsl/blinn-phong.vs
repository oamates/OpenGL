#version 330 core

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;

uniform mat4 projection_view_matrix;

uniform vec3 camera_ws;
uniform vec3 light_ws;

out vec3 view_direction;
out vec3 light_direction;
out vec3 position;
out vec3 normal;

void main()
{
    position = position_in;
    normal = normal_in;

    view_direction = camera_ws - position_in;
    light_direction = light_ws - position_in;

    gl_Position = projection_view_matrix * vec4(position_in, 1.0f);
}