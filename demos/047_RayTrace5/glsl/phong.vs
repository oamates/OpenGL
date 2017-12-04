#version 450 core

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;

out vec3 normal_cs;
out vec3 normal_ws;
out vec3 position_cs;
out vec3 position_ws;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

void main()
{
    vec4 position_ws4 = M * vec4(position_in, 1.0f);
    position_ws = vec3(position_ws4);

    vec4 position_cs4 = V * position_ws4;
    position_cs = vec3(position_cs4);

    vec4 normal_ws4 = M * vec4(normal_in, 0);
	normal_ws = vec3(normal_ws4);

	vec4 normal_cs4 = V * normal_ws4;
    normal_cs = vec3(normal_cs4);

	gl_Position = P * position_cs4;
}



