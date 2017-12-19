#version 330 core

uniform mat4 TransformMatrix;

in vec4 Position;
out vec3 vertPosition;

void main()
{
    vertPosition = Position.xyz;
    gl_Position = TransformMatrix * Position;
}