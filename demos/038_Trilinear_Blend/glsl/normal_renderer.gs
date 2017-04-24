#version 330 core
layout(triangles) in;
     
layout(line_strip, max_vertices = 9) out;

uniform mat4 projection_view_matrix;

in vec4 position_ws[];
in vec4 normal_ws[];
     
void main()
{
    const float l = 0.125;

    vec4 glposA0 = projection_view_matrix * position_ws[0];
    vec4 glposA1 = projection_view_matrix * (position_ws[0] + l * normal_ws[0]);

    vec4 glposB0 = projection_view_matrix * position_ws[1];
    vec4 glposB1 = projection_view_matrix * (position_ws[1] + l * normal_ws[1]);

    vec4 glposC0 = projection_view_matrix * position_ws[2];
    vec4 glposC1 = projection_view_matrix * (position_ws[2] + l * normal_ws[2]);

    gl_Position = glposA1;
    EmitVertex();            
    gl_Position = glposA0;
    EmitVertex();            
    gl_Position = glposB0;
    EmitVertex();
    EndPrimitive();
                
    gl_Position = glposB1;
    EmitVertex();            
    gl_Position = glposB0;
    EmitVertex();            
    gl_Position = glposC0;
    EmitVertex();
    EndPrimitive();
                
    gl_Position = glposC1;
    EmitVertex();            
    gl_Position = glposC0;
    EmitVertex();            
    gl_Position = glposA0;
    EmitVertex();
    EndPrimitive();
}