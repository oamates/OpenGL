#version 330 core

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;
layout(location = 2) in vec3 tangent_x_in;
layout(location = 3) in vec3 tangent_y_in;

uniform mat4 projection_view_matrix;

out vec3 position_ws;

void main()
{
    position_ws = position_in;
	gl_Position = projection_view_matrix * vec4(position_in, 1.0f);
}


