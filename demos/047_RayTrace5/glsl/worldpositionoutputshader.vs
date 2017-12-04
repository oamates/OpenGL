#version 410 core

layout(location = 0) in vec3 position_in;
out vec3 position_ws;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

void main()
{
    vec4 position_ws4 = M * vec4(position_in, 1.0f);
	position_ws = vec3(position_ws4);
	gl_Position = P * V * position_ws4;
}



