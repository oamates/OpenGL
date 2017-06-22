#version 430 core

in vec4 position_in;
in vec4 normal_in;

uniform mat4 projection_view_matrix;

out vec3 position_ws_gs;
out vec3 normal_ws_gs;

void main(void)
{
	position_ws_gs = position_in.xyz;
	normal_ws_gs = normal_in.xyz;
}