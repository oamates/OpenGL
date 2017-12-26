#version 330 core

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec3 Tangent;
layout(location = 3) in vec3 Bitangent;
layout(location = 4) in vec2 TexCoord;

uniform mat4 ModelMatrix;
uniform vec3 CameraPosition;
uniform vec3 LightPosition;

out gl_PerVertex 
{
   vec4 gl_Position;
};

out vec3 vertNormal;
out vec3 vertTangent;
out vec3 vertBitangent;
out vec3 vertLightDir;
out vec3 vertViewDir;
out vec2 vertTexCoord;
out vec2 vertSTCoord;

void main()
{
    gl_Position = ModelMatrix * vec4(Position, 1.0f);

    vertLightDir = LightPosition - gl_Position.xyz;
    vertViewDir = CameraPosition - gl_Position.xyz;
    vertNormal =  (ModelMatrix * vec4(Normal,  0.0f)).xyz;
    vertTangent = (ModelMatrix * vec4(Tangent, 0.0f)).xyz;
    vertBitangent = (ModelMatrix * vec4(Bitangent, 0.0)).xyz;
    vertTexCoord = TexCoord;
    vertSTCoord = TexCoord;
}
