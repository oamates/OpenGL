#version 330                                                                        

layout(location=0) in vec3 pos;
layout(location=1) in vec3 color;
layout(location=2) in vec2 texcoord;

out vec3 colorf;
out vec2 texcoordf;

uniform mat4 view_matrix;
uniform mat4 projection_matrix;
uniform mat4 model_matrix[512];

flat out int instanceID;


void main()
{
	texcoordf = texcoord;
	colorf = color;
	instanceID = gl_InstanceID;
	gl_Position = projection_matrix * view_matrix * model_matrix[gl_InstanceID] * vec4(pos, 1.0f);
}