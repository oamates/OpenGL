#version 330 core

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;

uniform mat4 model_matrix;
uniform mat4 projection_view_matrix;
uniform vec3 camera_ws;
uniform vec3 light_ws;

out vec3 position_ws;
out vec3 normal_ws;
out vec3 view;
out vec3 light;

void main()
{
    mat3 normal_matrix = transpose(inverse(mat3(model_matrix)));

    vec4 position_ws4 = model_matrix * vec4(16.0 * position_in, 1.0f);
    position_ws = vec3(position_ws4);
    normal_ws = normal_matrix * normal_in;

    view = camera_ws - position_ws;
    light = light_ws - position_ws;

    gl_Position = projection_view_matrix * position_ws4;
}