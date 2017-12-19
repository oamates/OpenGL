#version 330 core

layout(location = 0) in vec3 Position;

uniform mat4 LightMatrix;
uniform mat4 ModelMatrix;
uniform vec3 LightPosition;
uniform vec3 CameraPosition;

out vec4 vertDepthCoord;
out vec3 vertLightDir;
out vec3 vertViewDir;

void main()
{
    gl_Position = ModelMatrix * vec4(Position, 1.0f);
    vertDepthCoord = LightMatrix * gl_Position;
    vertLightDir = LightPosition - gl_Position.xyz;
    vertViewDir = CameraPosition - gl_Position.xyz;
}
