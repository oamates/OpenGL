#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;

out vec3 uvw;

uniform mat4 projection_view_matrix;
uniform int instance_count;

const float scale = 40.0f;
void main(void)
{
	float w = float(gl_InstanceID) / float(instance_count);
	uvw = vec3(uv, w);
	vec3 position_ws = scale * vec3(position, 2.0f * w - 1.0f);
    gl_Position = projection_view_matrix * vec4(position_ws, 1.0f);
}