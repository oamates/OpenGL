#version 430 core

layout(location = 0) uniform mat4 View;
layout(location = 1) uniform mat4 Projection;
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

out vec2 txcoord;

void main() 
{
	vec4 pos = View * gl_in[0].gl_Position;
	txcoord = vec2(-1.0f, -1.0f);
	gl_Position = Projection * (pos + 0.2f * vec4(txcoord, 0.0f, 0.0f));
	EmitVertex();
	txcoord = vec2( 1.0f, -1.0f);
	gl_Position = Projection * (pos + 0.2f * vec4(txcoord, 0.0f, 0.0f));
	EmitVertex();
	txcoord = vec2(-1.0f,  1.0f);
	gl_Position = Projection * (pos + 0.2f * vec4(txcoord, 0.0f, 0.0f));
	EmitVertex();
	txcoord = vec2( 1.0f,  1.0f);
	gl_Position = Projection * (pos + 0.2f * vec4(txcoord, 0.0f, 0.0f));
	EmitVertex();
};
