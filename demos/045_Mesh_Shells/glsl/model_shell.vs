#version 330 core

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec3 normal_in;

out vec3 position;
out vec3 normal;
out vec3 color_mark;

void main()
{
    switch(gl_VertexID)
    {
        case 5      : color_mark = vec3(0.1, 0.1, 0.1); break;
        case 6      : color_mark = vec3(0.1, 0.1, 0.1); break;
        case 7      : color_mark = vec3(0.1, 0.1, 0.1); break;
        case 8      : color_mark = vec3(0.1, 0.1, 0.1); break;
        case 19     : color_mark = vec3(1.0, 0.0, 0.0); break;
        case 24     : color_mark = vec3(0.0, 1.0, 0.0); break;
        case 292722 : color_mark = vec3(0.0, 0.0, 1.0); break;
        default: color_mark = vec3(0.0);
    }

	normal = normal_in;
	position = position_in;
}