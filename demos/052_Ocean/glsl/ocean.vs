#version 330 core

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;
//layout(location = 2) in vec2 htilde0_in;
//layout(location = 3) in vec2 htilde1_in;
layout(location = 2) in vec2 uv_in;

uniform mat4 projection_view_matrix;
uniform mat4 model_matrix;

out vec3 position_ws;
out vec3 normal_ws;
out vec2 uv;

void main()
{
	vec4 position_ws4 = model_matrix * vec4(position_in, 1.0);
	position_ws = vec3(position_ws4);
    normal_ws = inverse(transpose(mat3(model_matrix))) * normal_in;
	gl_Position = projection_view_matrix * position_ws4;
	uv = uv_in;
}