#version 430 core

in vec4 position;

uniform mat4 projection_view_matrix;

out float intensity;

void main(void)
{
    intensity = position.w;
    gl_Position = projection_view_matrix * vec4(position.xyz, 1.0); 
}