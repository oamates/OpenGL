#version 330 core

uniform mat4 view_matrix;
uniform mat4 projection_matrix;
 
layout (location = 0) in vec3 position_in;
layout (location = 3) in vec2 uv_in;

out float Depth;
out float Z;
out vec2 uv;
 
void main(void)
{
	uv = uv_in;

    vec4 position_cs = view_matrix * vec4(position_in, 1.0);
	vec4 depthVec = projection_matrix * position_cs;
	Depth = 0.5f + 0.5f * (depthVec.z / depthVec.w); 
	Z = position_cs.z;
	gl_Position = depthVec;
}