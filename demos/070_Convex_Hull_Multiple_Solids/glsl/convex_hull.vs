#version 330 core

layout(location = 0) in vec3 position_ms;
layout(location = 1) in vec3 normal_ms;

uniform mat4 projection_view_matrix;
uniform mat4 model_matrix;

out vec3 position_ws;
out vec3 normal_ws;

void main()
{
    vec4 position_ws4 = model_matrix * vec4(position_ms, 1.0f);
    position_ws = vec3(position_ws4);

    normal_ws = mat3(model_matrix) * normal_ms;
	gl_Position = projection_view_matrix * position_ws4;
}


