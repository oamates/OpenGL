#version 400 core

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;
layout(location = 2) in vec2 uv_in;

uniform mat4 projection_view_matrix;

out vec3 position_ws;
out vec3 normal_ws;
out vec2 uv;

void main()
{
    position_ws = position_in;
    normal_ws = normal_in;
    uv = uv_in;
	gl_Position = projection_view_matrix * vec4(position_ws, 1.0f);
}
