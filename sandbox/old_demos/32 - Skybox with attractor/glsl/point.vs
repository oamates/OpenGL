#version 330 core

layout(location = 0) in vec3 point3d;  // incoming vertices

uniform mat4 projection_view_matrix;

void main()
{
	vec4 point4d = vec4(25.0f * point3d, 1.0f);

	gl_Position = projection_view_matrix * point4d;
    gl_PointSize = 2;

}


