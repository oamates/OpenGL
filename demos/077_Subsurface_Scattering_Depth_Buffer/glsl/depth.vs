#version 330 core

layout(location = 0) in vec3 Position;

uniform mat4 CameraMatrix;
uniform mat4 ModelMatrix;

void main(void)
{
    gl_Position = CameraMatrix * ModelMatrix * vec4(Position, 1.0f);
}
