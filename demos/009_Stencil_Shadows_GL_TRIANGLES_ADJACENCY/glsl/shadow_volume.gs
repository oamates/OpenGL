#version 330 core

layout (triangles_adjacency) in;                                    // six vertices in
layout (triangle_strip, max_vertices = 18) out;

in vec3 position_ws[];

uniform vec3 light_ws;
uniform mat4 projection_view_matrix;

const float lbias = 0.035625;
const float ibias = 10000.0125;

void main()
{
    vec3 e02 = position_ws[2] - position_ws[0];                     // vector AB
    vec3 e24 = position_ws[4] - position_ws[2];                     // vector BC
    vec3 e40 = position_ws[0] - position_ws[4];                     // vector CA



    vec3 normal = cross(e02, e24);

    // handle only light facing triangles
    if (dot(position_ws[0] - light_ws, normal) > 0) return;                                


    vec3 l0 = normalize(position_ws[0] - light_ws);                 // from light to A
    vec3 l2 = normalize(position_ws[2] - light_ws);                 // from light to B
    vec3 l4 = normalize(position_ws[4] - light_ws);                 // from light to C

    vec3 e01 = position_ws[1] - position_ws[0];
    vec3 e23 = position_ws[3] - position_ws[2];
    vec3 e45 = position_ws[5] - position_ws[4];

    // six vertices of the infinite prism
    vec4 v0l = projection_view_matrix * vec4(position_ws[0] + lbias * l0, 1.0f);
    vec4 v2l = projection_view_matrix * vec4(position_ws[2] + lbias * l2, 1.0f);
    vec4 v4l = projection_view_matrix * vec4(position_ws[4] + lbias * l4, 1.0f);

    vec4 v0i = projection_view_matrix * vec4(l0, 0.0f);
    vec4 v2i = projection_view_matrix * vec4(l2, 0.0f);
    vec4 v4i = projection_view_matrix * vec4(l4, 0.0f);

    // emit the triangle itself, slightly biased toward infinity from light position
    gl_Position = v0l; EmitVertex();
    gl_Position = v2l; EmitVertex();
    gl_Position = v4l; EmitVertex();
    EndPrimitive();

    normal = cross(e01, e02);                                       // test the triangle adjacent to AB edge
    if ((dot(normal, l0) >= 0))
    {
        gl_Position = v0l; EmitVertex();
        gl_Position = v0i; EmitVertex();
        gl_Position = v2l; EmitVertex();
        gl_Position = v2i; EmitVertex();
        EndPrimitive();
    }

    normal = cross(e23, e24);                                       // test the triangle adjacent to BC edge
    if ((dot(normal, l2) >= 0))
    {
        gl_Position = v2l; EmitVertex();
        gl_Position = v2i; EmitVertex();
        gl_Position = v4l; EmitVertex();
        gl_Position = v4i; EmitVertex();
        EndPrimitive();
    }

    normal = cross(e45, e40);                                       // test the triangle adjacent to CA edge
    if ((dot(normal, l4) >= 0))
    {
        gl_Position = v4l; EmitVertex();
        gl_Position = v4i; EmitVertex();
        gl_Position = v0l; EmitVertex();
        gl_Position = v0i; EmitVertex();
        EndPrimitive();
    }

    // render the projective triangle at infinity
    gl_Position = v4i; EmitVertex();
    gl_Position = v2i; EmitVertex();
    gl_Position = v0i; EmitVertex();
    EndPrimitive();
}