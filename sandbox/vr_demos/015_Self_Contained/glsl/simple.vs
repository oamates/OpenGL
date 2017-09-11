#version 410 core

uniform mat4 ProjectionMatrix;
uniform mat4 CameraMatrix;

layout(location = 0) in vec4 Position;
layout(location = 2) in vec3 Normal;
layout(location = 5) in mat4 InstanceTransform;

out vec3 vertNormal;

void main(void)
{
    mat4 ViewXfm = CameraMatrix * InstanceTransform;
    vertNormal = Normal;
    gl_Position = ProjectionMatrix * ViewXfm * Position;
}