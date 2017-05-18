#version 430 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

const float znear = 0.5f;
const float zn2 = 2.0f * znear;

out int gl_Layer;

in vec3 position_ls[3];
                  
void main()
{
    // GL_CUBE_MAP_POSITIVE_X :: (x, y, z, 1) --> (z, y, -x, 1) --> (z, y, x-1, x)
    gl_Layer = 0;
    for (int i = 0; i < 3; ++i)
    {
        gl_Position = vec4(-position_ls[i].z, -position_ls[i].y, position_ls[i].x - zn2, position_ls[i].x);
        EmitVertex();
    }
    EndPrimitive();

    // GL_CUBE_MAP_NEGATIVE_X :: (x, y, z, 1) --> (-z, y, x, 1) --> (-z, y, -x-1, -x)
    gl_Layer = 1;                                
    for (int i = 0; i < 3; ++i)
    {
        gl_Position = vec4(position_ls[i].z, -position_ls[i].y, -position_ls[i].x - zn2, -position_ls[i].x);
        EmitVertex();
    }
    EndPrimitive();

    // GL_CUBE_MAP_POSITIVE_Y :: (x, y, z, 1) --> (-x, -z, -y, 1) --> (-x, -z, y-1, y)
    gl_Layer = 2;                                
    for (int i = 0; i < 3; ++i)
    {
        gl_Position = vec4(position_ls[i].x, position_ls[i].z, position_ls[i].y - zn2, position_ls[i].y);
        EmitVertex();
    }
    EndPrimitive();

    // GL_CUBE_MAP_NEGATIVE_Y :: (x, y, z, 1) --> (-x, z, y, 1) --> (-x, z, -y-1, -y)
    gl_Layer = 3;                                
    for (int i = 0; i < 3; ++i)
    {
        gl_Position = vec4(position_ls[i].x, -position_ls[i].z, -position_ls[i].y - zn2, -position_ls[i].y);
        EmitVertex();
    }
    EndPrimitive();

    // GL_CUBE_MAP_POSITIVE_Z :: (x, y, z, 1) --> (-x, y, -z, 1) --> (-x, y, z-1, z)
    gl_Layer = 4;                                
    for (int i = 0; i < 3; ++i)
    {
        gl_Position = vec4(position_ls[i].x, -position_ls[i].y, position_ls[i].z - zn2, position_ls[i].z);
        EmitVertex();
    }
    EndPrimitive();

    // GL_CUBE_MAP_NEGATIVE_Z :: (x, y, z, 1) --> (x, y, z, 1) --> (x, y, -z-1, -z)
    gl_Layer = 5;                                
    for (int i = 0; i < 3; ++i)
    {
        gl_Position = vec4(-position_ls[i].x, -position_ls[i].y, -position_ls[i].z - zn2, -position_ls[i].z);
        EmitVertex();
    }
    EndPrimitive();
}
























