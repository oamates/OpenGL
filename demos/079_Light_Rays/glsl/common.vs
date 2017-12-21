#version 330 core

in vec4 Position;
in vec3 Normal;

uniform vec3 LightPosition;
uniform mat4 ProjectionMatrix, CameraMatrix, ModelMatrix, LightMatrix;

out vec3 vertNormal;
out vec3 vertLightDir;
out vec4 vertShadowCoord;

void main()
{
    gl_Position = ModelMatrix * Position;
    vertLightDir = normalize(LightPosition - gl_Position.xyz);
    vertNormal = normalize(mat3(ModelMatrix) * Normal);
    vertShadowCoord = LightMatrix * ModelMatrix * Position;
    gl_Position = ProjectionMatrix * CameraMatrix * gl_Position;
}
