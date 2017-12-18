#version 330 core

layout(location = 0) in vec3 position_ms;
layout(location = 1) in vec3 normal_ms;

uniform mat4 model_matrix;

out vec4 position_ws4;
out vec4 normal_ws4;

void main()
{
    position_ws4 = model_matrix * vec4(position_ms, 1.0f);
    normal_ws4 = model_matrix * vec4(normal_ms, 0.0f);
}

