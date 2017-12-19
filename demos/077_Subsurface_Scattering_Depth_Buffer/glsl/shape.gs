#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec4 vertDepthCoord[3];
in vec3 vertLightDir[3];
in vec3 vertViewDir[3];

uniform mat4 CameraMatrix;

out vec4 geomDepthCoord;
out vec3 geomLightDir;
out vec3 geomViewDir;
out vec3 geomNormal;

void main()
{
    geomNormal = normalize(
        cross(
            gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz,
            gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz
        )
    );

    for(int v = 0; v != 3; ++v)
    {
        gl_Position = CameraMatrix * gl_in[v].gl_Position;
        geomDepthCoord = vertDepthCoord[v];
        geomLightDir = vertLightDir[v];
        geomViewDir = vertViewDir[v];
        EmitVertex();
    }
    EndPrimitive();
}

