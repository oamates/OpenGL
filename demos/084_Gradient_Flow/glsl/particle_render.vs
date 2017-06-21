#version 430 core

in vec4 position_in;

uniform mat4 projection_view_matrix;

out vec3 position_ws;
out float intensity;

void main(void)
{
	position_ws = position_in.xyz;
    intensity = position_in.w;
    gl_Position = projection_view_matrix * vec4(position_ws, 1.0f); 
}