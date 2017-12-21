#version 330 core

in vec4 Position;
in vec3 Normal;
in vec2 TexCoord;

uniform mat4 ProjectionMatrix;
uniform mat4 CameraMatrix;
uniform mat4 ModelMatrix;
uniform vec3 LightPos;

out vec3 vertNormal;
out vec3 vertLightDir;
out vec3 vertLightRefl;
out vec3 vertViewDir;
out vec3 vertViewRefl;

void main()
{
    gl_Position = ModelMatrix * Position;
    vertNormal = mat3(ModelMatrix) * Normal;
    vertLightDir = LightPos - gl_Position.xyz;
    vertLightRefl = reflect(-normalize(vertLightDir), normalize(vertNormal));
    vertViewDir = (vec4(0.0, 0.0, 1.0, 1.0) * CameraMatrix).xyz;
    vertViewRefl = reflect(normalize(vertViewDir), normalize(vertNormal));
    gl_Position = ProjectionMatrix * CameraMatrix * gl_Position;
}
