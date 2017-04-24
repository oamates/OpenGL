#version 330 core

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;
layout(location = 2) in vec3 tangent_x_in;
layout(location = 3) in vec3 tangent_y_in;
layout(location = 4) in vec2 uv_in;

uniform mat4 projection_view_matrix;
uniform mat4 model_matrix;

out vec3 position_ws;
out vec3 normal_ws;
out vec3 tangent_x_ws;
out vec3 tangent_y_ws;
out vec2 uv;

void main()
{
    mat3 rotation_matrix = mat3(model_matrix);
	vec4 position_ws4 = model_matrix * vec4(position_in, 1.0f);

    position_ws  = vec3(position_ws4);
	normal_ws    = rotation_matrix * normal_in;
    tangent_x_ws = rotation_matrix * tangent_x_in;
    tangent_y_ws = rotation_matrix * tangent_y_in;
    uv = uv_in;

	gl_Position = projection_view_matrix * position_ws4;
}