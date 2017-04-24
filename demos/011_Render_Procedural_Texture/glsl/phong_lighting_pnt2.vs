#version 330 core

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;
layout(location = 2) in vec2 uv_in;

uniform mat4 projection_view_matrix;
uniform vec3 camera_ws;
uniform vec3 light_ws;

out vec3 position_ws;
out vec3 normal_ws;
out vec2 uv;

void main()
{
	gl_Position = projection_view_matrix * vec4(position_in, 1.0f);
	uv = uv_in;
	normal_ws = normal_in;
	position_ws = position_in;
}