#version 430 core

layout (local_size_x = 128) in;

layout (r32i, binding = 0) uniform iimageBuffer index_buffer;
layout (rg32f, binding = 1) uniform imageBuffer vertex_buffer;

uniform int shift;
uniform int triangles;
uniform vec3 camera_ws;

vec3 position(int idx)
{
    vec2 xy = imageLoad(vertex_buffer, idx).xy;
    float z = imageLoad(vertex_buffer, idx + 1).x;
    return vec3(xy, z);
}

const float one_third = 1.0 / 3.0;

void main()
{
    int id0 = 2 * int(gl_GlobalInvocationID.x) + shift;

    if (id0 + 1 >= triangles) return;

    int base_index = 3 * id0;

    int iA0 = imageLoad(index_buffer, base_index + 0).x;
    int iB0 = imageLoad(index_buffer, base_index + 1).x;
    int iC0 = imageLoad(index_buffer, base_index + 2).x;
    int iA1 = imageLoad(index_buffer, base_index + 3).x;
    int iB1 = imageLoad(index_buffer, base_index + 4).x;
    int iC1 = imageLoad(index_buffer, base_index + 5).x;

    vec3 A0 = position(3 * iA0);
    vec3 B0 = position(3 * iB0);
    vec3 C0 = position(3 * iC0);
    vec3 A1 = position(3 * iA1);
    vec3 B1 = position(3 * iB1);
    vec3 C1 = position(3 * iC1);

    vec3 center0 = one_third * (A0 + B0 + C0);
    vec3 center1 = one_third * (A1 + B1 + C1);

    float d0 = length(center0 - camera_ws); 
    float d1 = length(center1 - camera_ws); 

    if (d1 > d0)
    {
        imageStore(index_buffer, base_index + 0, ivec4(iA1, 0, 0, 0));
        imageStore(index_buffer, base_index + 1, ivec4(iB1, 0, 0, 0));
        imageStore(index_buffer, base_index + 2, ivec4(iC1, 0, 0, 0));
        imageStore(index_buffer, base_index + 3, ivec4(iA0, 0, 0, 0));
        imageStore(index_buffer, base_index + 4, ivec4(iB0, 0, 0, 0));
        imageStore(index_buffer, base_index + 5, ivec4(iC0, 0, 0, 0));
    }
}