#version 330 core

layout (triangles) in;
layout (points, max_vertices = 1) out; 

in vec3 mid_position[];
in vec3 mid_normal[];
 
uniform float sigma;

out vec4 cloud_point;


const float one_third = 1.0 / 3.0;

void main()
{
    vec3 position = one_third * (mid_position[0] + mid_position[1] + mid_position[2]);
    vec3 normal = normalize(mid_normal[0] + mid_normal[1] + mid_normal[2]);
    
    cloud_point = vec4(position + sigma * normal, 0.0);
    EmitVertex();
}