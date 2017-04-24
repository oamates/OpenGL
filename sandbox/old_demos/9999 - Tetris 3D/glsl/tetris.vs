#version 330                                                                        

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 texcoord;

out vec3 texcoordf;
flat out int instanceID;

uniform mat4 view_matrix;
uniform mat4 projection_matrix;
uniform vec3 shift[1024];


void main()
{
	texcoordf = texcoord;
	instanceID = gl_InstanceID;
	gl_Position = projection_matrix * view_matrix * vec4(shift[gl_InstanceID] + position, 1.0f);
}