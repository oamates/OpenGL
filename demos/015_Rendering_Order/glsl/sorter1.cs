#version 430 core

layout (local_size_x = 128) in;

layout (r32i, binding = 0) uniform iimageBuffer index_buffer;
layout (rgba32f, binding = 1) uniform imageBuffer position_ws_buffer;

uniform vec3 camera_ws;

void main(void)
{
    if (gl_GlobalInvocationID.x == 0) return;
    int id0 = 2 * int(gl_GlobalInvocationID.x) - 1;
    int id1 = id0 + 1;

    int index0 = imageLoad(index_buffer, id0).x;
    int index1 = imageLoad(index_buffer, id1).x;

    vec3 position_ws0 = imageLoad(position_ws_buffer, index0).xyz;
    vec3 position_ws1 = imageLoad(position_ws_buffer, index1).xyz;

    float d0 = length(position_ws0 - camera_ws); 
    float d1 = length(position_ws1 - camera_ws); 

    if (d1 < d0)
    {
        imageStore(index_buffer, id0, ivec4(index1, 0, 0, 0));
        imageStore(index_buffer, id1, ivec4(index0, 0, 0, 0));
    }
}