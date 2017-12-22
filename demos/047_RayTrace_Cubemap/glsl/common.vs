#version 330 core

uniform mat4 ModelMatrix;
uniform mat3 TextureMatrix;
uniform vec3 CameraPosition;
uniform vec3 LightPosition;

in vec4 Position;
in vec3 Normal;
in vec3 Tangent;
in vec2 TexCoord;

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
    gl_Position = ModelMatrix * Position;
    vertLightDir = LightPosition - gl_Position.xyz;
    vertViewDir = CameraPosition - gl_Position.xyz;
    vertNormal =  (ModelMatrix * vec4(Normal,  0.0)).xyz;
    vertTangent = (ModelMatrix * vec4(Tangent, 0.0)).xyz;
    vertBitangent = cross(vertNormal, vertTangent);
    vertTexCoord = (TextureMatrix * vec3(TexCoord,1.0)).xy;
    vertSTCoord = TexCoord;
}
