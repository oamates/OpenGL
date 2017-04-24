#version 430 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 uv;

uniform mat4 projection_view_matrix;
uniform float dt;
uniform float time;

layout (std430, binding = 0, shared) buffer shader_data
{
	vec4 attractor[64];
};


out vec3 position_ws;
out vec3 normal_ws;
out vec3 texture_coords;

void main(void)
{
	vec3 shift = vec3(attractor[gl_InstanceID]);
	float hole_radius = attractor[gl_InstanceID].w;
    position_ws = shift + hole_radius * position;
    normal_ws = normal;
    gl_Position = projection_view_matrix * vec4(position_ws, 1.0f);
    texture_coords = uv;
}