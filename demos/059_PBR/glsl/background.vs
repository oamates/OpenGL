#version 330 core

layout (location = 0) in vec3 position_in;

uniform mat4 projection;
uniform mat4 view;

out vec3 position_ws;

void main()
{
    position_ws = position_in;
	vec4 position_clip = projection * vec4(mat3(view) * position_in, 1.0f);
	gl_Position = position_clip.xyww;
}