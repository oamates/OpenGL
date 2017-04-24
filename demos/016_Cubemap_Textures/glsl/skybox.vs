#version 330 core

layout (location = 0) in vec3 position;

uniform mat4 projection_view_matrix;

out vec3 uvw;

void main(void)
{
    uvw = position;
    gl_Position = projection_view_matrix * vec4(1000.0f * position, 1.0f);
}


