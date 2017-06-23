#version 330 core

layout(location = 0) in vec4 position_in;
layout(location = 1) in vec3 axis_x_in;
layout(location = 2) in vec3 axis_y_in;
layout(location = 3) in vec3 axis_z_in;

out vec4 position_ws;
out vec3 axis_x;
out vec3 axis_y;
out vec3 axis_z;

void main()
{
	position_ws = position_in;

	axis_x = axis_x_in;
	axis_y = axis_y_in;
	axis_z = axis_z_in;
}


