#version 330 core

layout(location = 0) in vec3 position_ms;
layout(location = 1) in vec3 normal_ms;

uniform mat4 projection_matrix;
uniform mat4 view_matrix;
uniform mat4 model_matrix;

out vec4 position_ws;
out vec4 normal_ws;

void main()
{

    position_ws = model_matrix * vec4(position_ms, 1.0f);
    normal_ws = model_matrix * vec4(normal_ms, 0.0f);
}

