#version 330 core

layout (points) in;
layout (triangle_strip, max_vertices = 24) out;

const int indices[24] = int[24] ( 5, 7, 1, 3, 0, 2, 4, 6, 6, 2, 7, 3, 0, 4, 1, 5, 4, 6, 5, 7, 1, 3, 0, 2 );

const vec3 cube_vertex[8] = vec3[8]
(
    vec3(-1.0f, -1.0f, -1.0f),
    vec3( 1.0f, -1.0f, -1.0f),
    vec3(-1.0f,  1.0f, -1.0f),
    vec3( 1.0f,  1.0f, -1.0f),
    vec3(-1.0f, -1.0f,  1.0f),
    vec3( 1.0f, -1.0f,  1.0f),
    vec3(-1.0f,  1.0f,  1.0f),
    vec3( 1.0f,  1.0f,  1.0f)
);

const vec2 uvs[4] = vec2[4]
(
    vec2(-1.0f,  1.0f),
    vec2(-1.0f, -1.0f),
    vec2( 1.0f,  1.0f),
    vec2( 1.0f, -1.0f)
);

out vec3 ray;

void main()
{
    int idx = 0;
    for(gl_Layer = 0; gl_Layer < 6; ++gl_Layer)
    {
        for(int v = 0; v < 4; ++v)
        {
            vec2 uv = uvs[v];
            ray = cube_vertex[indices[idx++]];
            gl_Position = vec4(uv, 1.0f, 1.0f);
            EmitVertex();
        }
        EndPrimitive();
    }
}
