#version 330 core

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;

uniform mat4 model_matrix;

out vec4 position_ws;
out vec4 normal_ws;

void main()
{
    mat3 normal_matrix = transpose(inverse(mat3(model_matrix)));

    position_ws = model_matrix * vec4(position_in, 1.0f);
    normal_ws = vec4(normal_matrix * normal_in, 0.0f);
}

