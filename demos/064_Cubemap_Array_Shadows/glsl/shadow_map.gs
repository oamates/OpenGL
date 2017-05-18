#version 430 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 72) out;

const int LIGHT_COUNT = 4;

uniform vec3 light_ws[LIGHT_COUNT];

const float znear = 0.5f;
const float zn2 = 2.0f * znear;

out int gl_Layer;

in vec3 position_ws[3];
                  
void main()
{
    int layer = 0;

    for(int l = 0; l < LIGHT_COUNT; ++l)
    {
        vec3 position_ls[3];

        for(int j = 0; j < 3; ++j)
            position_ls[j] = position_ws[j] - light_ws[l];

        // GL_CUBE_MAP_POSITIVE_X :: (x, y, z, 1) --> (z, y, -x, 1) --> (z, y, x-1, x)
        gl_Layer = layer++;
        for (int i = 0; i < 3; ++i)
        {
            gl_Position = vec4(-position_ls[i].z, -position_ls[i].y, position_ls[i].x - zn2, position_ls[i].x);
            EmitVertex();
        }
        EndPrimitive();

        // GL_CUBE_MAP_NEGATIVE_X :: (x, y, z, 1) --> (-z, y, x, 1) --> (-z, y, -x-1, -x)
        gl_Layer = layer++;                                
        for (int i = 0; i < 3; ++i)
        {
            gl_Position = vec4(position_ls[i].z, -position_ls[i].y, -position_ls[i].x - zn2, -position_ls[i].x);
            EmitVertex();
        }
        EndPrimitive();

        // GL_CUBE_MAP_POSITIVE_Y :: (x, y, z, 1) --> (-x, -z, -y, 1) --> (-x, -z, y-1, y)
        gl_Layer = layer++;                                
        for (int i = 0; i < 3; ++i)
        {
            gl_Position = vec4(position_ls[i].x, position_ls[i].z, position_ls[i].y - zn2, position_ls[i].y);
            EmitVertex();
        }
        EndPrimitive();

        // GL_CUBE_MAP_NEGATIVE_Y :: (x, y, z, 1) --> (-x, z, y, 1) --> (-x, z, -y-1, -y)
        gl_Layer = layer++;                           
        for (int i = 0; i < 3; ++i)
        {
            gl_Position = vec4(position_ls[i].x, -position_ls[i].z, -position_ls[i].y - zn2, -position_ls[i].y);
            EmitVertex();
        }
        EndPrimitive();

        // GL_CUBE_MAP_POSITIVE_Z :: (x, y, z, 1) --> (-x, y, -z, 1) --> (-x, y, z-1, z)
        gl_Layer = layer++;                           
        for (int i = 0; i < 3; ++i)
        {
            gl_Position = vec4(position_ls[i].x, -position_ls[i].y, position_ls[i].z - zn2, position_ls[i].z);
            EmitVertex();
        }
        EndPrimitive();

        // GL_CUBE_MAP_NEGATIVE_Z :: (x, y, z, 1) --> (x, y, z, 1) --> (x, y, -z-1, -z)
        gl_Layer = layer++;                           
        for (int i = 0; i < 3; ++i)
        {
            gl_Position = vec4(-position_ls[i].x, -position_ls[i].y, -position_ls[i].z - zn2, -position_ls[i].z);
            EmitVertex();
        }
        EndPrimitive();

    }    
}
























