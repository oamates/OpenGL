#version 330 core
layout(triangles) in;
     
layout(line_strip, max_vertices = 10) out;

uniform mat4 projection_matrix;
uniform mat4 view_matrix;
uniform mat4 model_matrix;

in vec4 position_ws[];
in vec4 normal_ws[];
     
const float normal_length = 2.5f;
     
out vec4 vertex_color;

const vec4 color0 = vec4(0.0, 0.0, 0.0, 1.0);
const vec4 color1 = vec4(1.0, 0.0, 0.0, 1.0);
const vec4 colorM = vec4(0.0, 0.0, 1.0, 1.0);

void main()
{
        vec4 posA, posB, posC, glposA, glposB, glposC, posN;

        posA = position_ws[0];
        posN = position_ws[0] + normal_length * normal_ws[0];
        glposA = projection_matrix * view_matrix * posA;
        gl_Position = glposA;
        vertex_color = color0;
        EmitVertex();            
        gl_Position = projection_matrix * view_matrix * posN;
        vertex_color = color1;
        EmitVertex();
        EndPrimitive();

        posB = position_ws[1];
        posN = position_ws[1] + normal_length * normal_ws[1];
        glposB = projection_matrix * view_matrix * posB;
        gl_Position = glposB;
        vertex_color = color0;
        EmitVertex();            
        gl_Position = projection_matrix * view_matrix * posN;
        vertex_color = color1;
        EmitVertex();
        EndPrimitive();

        posC = position_ws[2];
        posN = position_ws[2] + normal_length * normal_ws[2];
        glposC = projection_matrix * view_matrix * posC;
        gl_Position = glposC;
        vertex_color = color0;
        EmitVertex();            
        gl_Position = projection_matrix * view_matrix * posN;
        vertex_color = color1;
        EmitVertex();
        EndPrimitive();

        gl_Position = glposA;
        vertex_color = colorM;
        EmitVertex();            
        gl_Position = glposB;
        vertex_color = colorM;
        EmitVertex();            
        gl_Position = glposC;
        vertex_color = colorM;
        EmitVertex();            
        gl_Position = glposA;
        vertex_color = colorM;
        EmitVertex();            
        EndPrimitive();
}