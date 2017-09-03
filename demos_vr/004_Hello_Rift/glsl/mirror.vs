#version 430 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

uniform mat4 projection_view_matrix;
uniform vec4 camera_ws;
uniform vec4 light_ws;

out vec3 light_direction;
out vec3 view_direction;
out vec3 normal_ws;
out vec2 texture_coord;

void main()
{
	vec4 position_ws = vec4(position, 1.0f);
	gl_Position = projection_view_matrix * position_ws;
    light_direction = vec3(light_ws - position_ws);
    view_direction = vec3(camera_ws - position_ws);    	
    normal_ws = normal;
	texture_coord = uv;
}


