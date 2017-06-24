#version 430 core

layout (local_size_x = 128) in;

layout (r32i, binding = 0) uniform iimageBuffer index_buffer;
layout (rgba32f, binding = 1) uniform imageBuffer position_ws_buffer;

uniform int stage;
uniform int pass;

void main()
{
    int id = int(gl_GlobalInvocationID.x);

    int pair_distance = 1 << (stage - pass);

    int l = id + (id & (-pair_distance));
    int r = l + pair_distance;
    
    int l_index = imageLoad(index_buffer, l).x;
    int r_index = imageLoad(index_buffer, r).x;

    vec3 position_ws0 = imageLoad(position_ws_buffer, l_index).xyz;
    vec3 position_ws1 = imageLoad(position_ws_buffer, r_index).xyz;

    float d0 = length(position_ws0);
    float d1 = length(position_ws1);

    bool correct_order = ((id >> stage) & 1) == 0;                          // true  -- increasing 
    bool actual_order = d0 > d1;                                            // false -- decreasing

    if (actual_order ^^ correct_order)
    {
        imageStore(index_buffer, l, ivec4(r_index, 0, 0, 0));
        imageStore(index_buffer, r, ivec4(l_index, 0, 0, 0));
    }
}