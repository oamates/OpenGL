#version 430 core


layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uvs;

layout (std430, binding = 0) buffer shader_data
{
	vec4 shift3d[];
};

uniform mat4 projection_view_matrix;

out vec3 normal_ws;
out vec2 texture_coords;
out float grey_scale;

void main()
{
	vec4 shift = shift3d[gl_InstanceID];
	grey_scale = shift.w;	
	shift.w = 0.0f;
	gl_Position = projection_view_matrix * (256.0f * shift + 2.65f * position);
	texture_coords = uvs;
	normal_ws = normal;
}