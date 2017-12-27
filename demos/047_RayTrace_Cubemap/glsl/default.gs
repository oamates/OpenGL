#version 330 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 view_matrix;
uniform mat4 projection_matrix;

in gl_PerVertex
{
    vec4 gl_Position;
} gl_in[3];

in vec3 vertNormal[3];
in vec3 vertTangent[3];
in vec3 vertBitangent[3];
in vec3 vertLightDir[3];
in vec3 vertViewDir[3];
in vec2 vertTexCoord[3];

out gl_PerVertex
{
    vec4 gl_Position;
};

out vec3 geomNormal;
out vec3 geomTangent;
out vec3 geomBitangent;
out vec3 geomLightDir;
out vec3 geomViewDir;
out vec2 geomTexCoord;

void main()
{
    for(int i = 0; i != 3; ++i)
    {
        gl_Position = projection_matrix * view_matrix * gl_in[i].gl_Position;
        geomNormal = vertNormal[i];
        geomTangent = vertTangent[i];
        geomBitangent = vertBitangent[i];
        geomLightDir = vertLightDir[i];
        geomViewDir = vertViewDir[i];
        geomTexCoord = vertTexCoord[i];
        EmitVertex();
    }
    EndPrimitive();
}
