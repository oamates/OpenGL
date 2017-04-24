#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color_in;

uniform float scale;
uniform mat4 projection_view_matrix;

out vec3 color;

void main()
{
    color = color_in;
    gl_Position = projection_view_matrix * vec4(scale * position, 1.0f);
}

