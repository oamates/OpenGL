#version 430 core

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;
layout(location = 2) in vec2 uv_in;

uniform mat4 projection_view_matrix;
uniform vec3 camera_ws;
uniform vec3 light_ws;

out vec3 view;
out vec3 light;
out vec3 normal_ws;
out vec2 uv;

void main()
{
    vec4 position_ws = vec4(position_in, 1.0f);

    view = vec3(camera_ws - position_in);
    light = vec3(light_ws - position_in);
    normal_ws = normal_in;
    
	gl_Position = projection_view_matrix * position_ws;
	uv = uv_in;
}


