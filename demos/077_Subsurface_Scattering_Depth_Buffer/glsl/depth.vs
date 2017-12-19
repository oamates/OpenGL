#version 330 core

in vec4 Position;

uniform mat4 CameraMatrix;
uniform mat4 ModelMatrix;

void main(void)
{
   gl_Position = CameraMatrix * ModelMatrix * Position;
}
