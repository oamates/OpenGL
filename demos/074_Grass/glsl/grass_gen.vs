#version 330 core

uniform ivec2 origin;

out flat ivec2 grid_point;

void main()
{
    grid_point = origin + ivec2(gl_VertexID, gl_InstanceID);
}


