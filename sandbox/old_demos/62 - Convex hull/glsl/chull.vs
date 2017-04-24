#version 330 core

layout(location = 0) in vec3 position;

uniform mat4 view_matrix;
uniform mat4 projection_matrix;

out vec3 position_ws;

out float vertexID;

void main()
{
	vertexID = float(gl_VertexID);
	position_ws = position;
    gl_Position = projection_matrix * view_matrix * vec4(position, 1.0f);
}


