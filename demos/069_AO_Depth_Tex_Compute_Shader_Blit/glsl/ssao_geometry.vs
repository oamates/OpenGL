#version 330 core

layout (location = 0) in vec3 position_in;

uniform mat4 pvm_matrix;

invariant gl_Position;

void main()
{
    gl_Position = pvm_matrix * vec4(position_in, 1.0f);
}