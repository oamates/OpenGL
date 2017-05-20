#version 330 core

uniform mat4 viewMatrix;
uniform mat4 projMatrix;
 
layout (location = 0) in vec3 in_position;
layout (location = 3) in vec2 in_texCoord;

out float Depth;
out float Z;
out vec2 TexCoord;
 
void main(void)
{
    vec4 position_cs = viewMatrix * vec4(in_position, 1.0);
	vec4 depthVec = projMatrix * position_cs;
	Depth = 0.5f + 0.5f * (depthVec.z / depthVec.w); 
	Z = position_cs.z;

	TexCoord = in_texCoord;

	gl_Position = depthVec;
}