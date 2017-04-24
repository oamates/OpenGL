#version 400                                                                        

layout(location = 0) in vec3 pos;

out vec3 position;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

void main()
{
	position = pos;
	gl_Position = projection_matrix * view_matrix * model_matrix * vec4(20.0f * pos, 1.0f);
}