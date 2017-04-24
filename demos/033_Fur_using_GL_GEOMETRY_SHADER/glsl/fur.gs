#version 430 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 240) out;

uniform mat4 view_matrix;
uniform mat4 projection_matrix;

uniform int fur_layers = 80;
uniform float fur_depth = 15.0;

in VS_GS_VERTEX
{
    vec3 normal;
    vec2 tex_coord;
} vertex_in[];

out GS_FS_VERTEX
{
    vec3 normal;
    vec2 tex_coord;
    flat float fur_strength;
} vertex_out;

void main()
{
    int i, layer;
    float disp_delta = 1.0 / float(fur_layers);
    float d = 0.0;
    vec4 position;

    for (layer = 0; layer < fur_layers; layer++)
    {
        for (i = 0; i < gl_in.length(); i++) 
        {
            vec3 n = vertex_in[i].normal;
            vertex_out.normal = n;
            vertex_out.tex_coord = vertex_in[i].tex_coord;
            vertex_out.fur_strength = 1.0 - d;
            position = gl_in[i].gl_Position + vec4(n * d * fur_depth, 0.0);
            gl_Position = projection_matrix * view_matrix * position;
            EmitVertex();
        };
        d += disp_delta;
        EndPrimitive();
    };
}
