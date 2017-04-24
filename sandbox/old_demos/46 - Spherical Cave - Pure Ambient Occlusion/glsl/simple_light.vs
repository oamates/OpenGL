#version 430 core

layout(location = 0) in vec3 position_ms;
layout(location = 1) in vec3 normal_ms;

uniform mat4 projection_matrix;
uniform mat4 view_matrix;
uniform mat4 model_matrix;

void main()
{

    vec4 position_ws = model_matrix * vec4(position_ms, 1.0f);
	gl_Position = projection_matrix * view_matrix * position_ws;

};


