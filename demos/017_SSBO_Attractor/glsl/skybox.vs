#version 430 core

layout(location = 0) in vec3 position;  // incoming vertices

uniform mat4 projection_view_matrix;
out vec3 position_ws;

void main()
{
	position_ws = position;
	gl_Position = projection_view_matrix * vec4(50000.0f * position, 1.0f);
}


