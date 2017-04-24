#version 430 core

layout(location = 0) in vec4 position_ms;
layout(location = 1) in vec3 normal_ms;
layout(location = 2) in vec2 texture_coord_v;

uniform mat4 projection_matrix;
uniform mat4 view_matrix;

out vec4 position;
out vec4 normal;
out vec4 view_direction;
out vec2 texture_coord_f;

void main()
{
    position = position_ms;
    normal = vec4(normal_ms, 0.0f);
    view_direction = view_matrix[3] - position;
	gl_Position = projection_matrix * view_matrix * position;
	texture_coord_f = texture_coord_v;
};


