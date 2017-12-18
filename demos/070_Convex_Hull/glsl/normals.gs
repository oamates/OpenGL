#version 330 core
layout(triangles) in;

layout(line_strip, max_vertices = 10) out;

uniform mat4 projection_view_matrix;

in vec4 position_ws4[];
in vec4 normal_ws4[];

const float normal_length = 2.5f;

out vec3 color;

const vec3 color_b = vec3(0.0, 1.0, 0.0);
const vec3 color_e = vec3(1.0, 1.0, 0.0);

void main()
{
    vec4 posA, posB, posC, glposA, glposB, glposC, posN;

    posA = position_ws4[0];
    posN = position_ws4[0] + normal_length * normal_ws4[0];
    glposA = projection_view_matrix * posA;
    gl_Position = glposA;
    color = color_b;
    EmitVertex();

    gl_Position = projection_view_matrix * posN;
    color = color_e;
    EmitVertex();
    EndPrimitive();

    posB = position_ws4[1];
    posN = position_ws4[1] + normal_length * normal_ws4[1];
    glposB = projection_view_matrix * posB;
    gl_Position = glposB;
    color = color_b;
    EmitVertex();

    gl_Position = projection_view_matrix * posN;
    color = color_e;
    EmitVertex();
    EndPrimitive();

    posC = position_ws4[2];
    posN = position_ws4[2] + normal_length * normal_ws4[2];
    glposC = projection_view_matrix * posC;
    gl_Position = glposC;
    color = color_b;
    EmitVertex();

    gl_Position = projection_view_matrix * posN;
    color = color_e;
    EmitVertex();
    EndPrimitive();

    gl_Position = glposA;
    color = color_b;
    EmitVertex();

    gl_Position = glposB;
    color = color_b;
    EmitVertex();

    gl_Position = glposC;
    color = color_b;
    EmitVertex();

    gl_Position = glposA;
    color = color_b;
    EmitVertex();
    EndPrimitive();
}