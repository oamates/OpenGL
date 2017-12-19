#version 330 core

in vec4 Position;

uniform mat4 LightMatrix;
uniform mat4 ModelMatrix;
uniform vec3 LightPosition;
uniform vec3 CameraPosition;

out vec4 vertDepthCoord;
out vec3 vertLightDir;
out vec3 vertViewDir;

void main()
{
   gl_Position = ModelMatrix * Position;
   vertDepthCoord = LightMatrix * gl_Position;
   vertLightDir = LightPosition - gl_Position.xyz;
   vertViewDir = CameraPosition - gl_Position.xyz;
}
