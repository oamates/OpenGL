#version 410 core

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;

out vec3 normal_viewspace;
out vec3 normal_worldspace;
out vec3 vertexPosition_viewspace;
out vec3 vertexPosition_worldspace;
out vec3 eyePosition_worldspace;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

void main()
{
	vertexPosition_viewspace = vec3(V * M * vec4(position_in, 1.0f));
	vertexPosition_worldspace = vec3(M * vec4(position_in, 1.0f));
	normal_viewspace = vec3(V * M * vec4(normal_in, 0.0f));
	normal_worldspace = vec3(M * vec4(normal_in, 0.0f));
	eyePosition_worldspace = vec3(inverse(V) * vec4(0.0f, 0.0f, 0.0f, 1.0f));
	gl_Position = P * V * M * vec4(position_in, 1.0f);
}