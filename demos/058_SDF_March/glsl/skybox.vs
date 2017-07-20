#version 330 core

layout (location = 0) in vec3 position_in;

uniform mat4 projection_view_matrix;

out vec3 uvw;

void main(void)
{
    uvw = position_in;
    gl_Position = projection_view_matrix * vec4(position_in, 0.0f);
}


