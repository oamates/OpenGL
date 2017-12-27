#version 330 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

uniform mat4 projection_matrix;
uniform mat4 view_matrix;

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

const mat4 face_matrix[6] = mat4[6]
(
    mat4( 0.0,  0.0, -1.0,  0.0,
          0.0, -1.0,  0.0,  0.0,
         -1.0,  0.0,  0.0,  0.0,
          0.0,  0.0,  0.0,  1.0),
    mat4( 0.0,  0.0,  1.0,  0.0,
          0.0, -1.0,  0.0,  0.0,
          1.0,  0.0,  0.0,  0.0,
          0.0,  0.0,  0.0,  1.0),
    mat4( 1.0,  0.0,  0.0,  0.0,
          0.0,  0.0, -1.0,  0.0,
          0.0,  1.0,  0.0,  0.0,
          0.0,  0.0,  0.0,  1.0),
    mat4( 1.0,  0.0,  0.0,  0.0,
          0.0,  0.0,  1.0,  0.0,
          0.0, -1.0,  0.0,  0.0,
          0.0,  0.0,  0.0,  1.0),
    mat4( 1.0,  0.0,  0.0,  0.0,
          0.0, -1.0,  0.0,  0.0,
          0.0,  0.0, -1.0,  0.0,
          0.0,  0.0,  0.0,  1.0),
    mat4(-1.0,  0.0,  0.0,  0.0,
          0.0, -1.0,  0.0,  0.0,
          0.0,  0.0,  1.0,  0.0,
          0.0,  0.0,  0.0,  1.0)
);

void main()
{
    for(gl_Layer = 0; gl_Layer != 6; ++gl_Layer)
    {
        mat4 TransformMatrix = projection_matrix * face_matrix[gl_Layer] * view_matrix;

        for(int i = 0; i != 3; ++i)
        {
            gl_Position = TransformMatrix * gl_in[i].gl_Position;
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
}