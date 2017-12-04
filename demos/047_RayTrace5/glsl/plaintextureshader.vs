#version 410 core

layout(location = 0) in vec3 position_in;

out vec2 uv;

void main()
{
	uv = 0.5 + 0.5 * position_in.xy;
	gl_Position = vec4(position_in, 1);
}



