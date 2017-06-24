#version 430 core

layout (local_size_x = 128) in;
layout (r32i, binding = 0) uniform iimageBuffer integral_data;

uniform int stage;
uniform int pass;

void main()
{
    int id = int(gl_GlobalInvocationID.x);

    int pair_distance = 1 << (stage - pass);

    int l = id + (id & (-pair_distance));
    int r = l + pair_distance;
    
    int l_elem = imageLoad(integral_data, l).x;
    int r_elem = imageLoad(integral_data, r).x;
    
    bool correct_order = ((id >> stage) & 1) == 0;                          // true  -- increasing 
    bool actual_order = l_elem < r_elem;                                    // false -- decreasing

    if (actual_order ^^ correct_order)
    {
        imageStore(integral_data, l, ivec4(r_elem, 0, 0, 0));
        imageStore(integral_data, r, ivec4(l_elem, 0, 0, 0));
    }
}
