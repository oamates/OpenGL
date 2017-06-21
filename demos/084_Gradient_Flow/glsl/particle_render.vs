#version 430 core

in vec4 position_in;
in vec4 normal_in;

uniform mat4 projection_view_matrix;

out vec3 position_ws;
out vec3 normal_ws;

void main(void)
{
	position_ws = position_in.xyz;
	normal_ws = normal_in.xyz;
    gl_Position = projection_view_matrix * vec4(4.0 * position_ws, 1.0f); 
}