#version 330 core

layout (points) in;
layout (triangle_strip, max_vertices = 24) out;

uniform mat4 projection_matrix;
uniform mat3 view_matrix;

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

out vec3 ray;

void main()
{
    int idx = 0;
    for(int f = 0; f < 6; ++f)
    {
        for(int v = 0; v < 4; ++v)
        {
            ray = cube_vertex[indices[idx++]];
            vec4 position_clip = projection_matrix * vec4(view_matrix * ray, 0.0f);
            gl_Position = position_clip;//.xyww;
            EmitVertex();
        }
        EndPrimitive();
    }
}