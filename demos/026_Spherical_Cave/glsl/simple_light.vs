#version 330 core

layout(location = 0) in vec3 position_ms;
layout(location = 1) in vec3 normal_ms;
layout(location = 2) in vec3 uv;

uniform mat4 projection_view_matrix;
uniform mat4 model_matrix;
uniform vec4 light_ws;
uniform vec4 camera_ws;

out vec4 position_ws;
out vec4 normal_ws;
out vec4 view_direction;
out vec4 light_direction;

void main()
{
    position_ws = model_matrix * vec4(position_ms, 1.0f);
    normal_ws = normalize(model_matrix * vec4(normal_ms, 0.0f));
    view_direction = camera_ws - position_ws;
    light_direction = light_ws - position_ws;
	gl_Position = projection_view_matrix * position_ws;
}


