#version 430 core

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

uniform uint triangles;

//==============================================================================================================================================================
// input  :: 
//   image unit 0 :: index buffer of GL_TRIANGLES type
//   image unit 1 :: positions buffer, vec3 with w component irrelevant
//==============================================================================================================================================================

layout (rgba32i, binding = 0) uniform iimageBuffer index_buffer;
layout (rgba32f, binding = 1) uniform imageBuffer vertex_buffer;

layout (r32ui, binding = 0) uniform uimage3D udf_tex;
layout (binding = 0, offset = 0) uniform atomic_int index_cntr;

//==============================================================================================================================================================
// unsigned distance from point to triangle function
// since it is going to be used millions of time it is inlined into the main body
//==============================================================================================================================================================

float dot2(vec3 v) 
    { return dot(v, v); }

float triangle_udf(vec3 p, vec3 a, vec3 b, vec3 c)
{
    vec3 AB = b - a; vec3 pa = p - a;
    vec3 BC = c - b; vec3 pb = p - b;
    vec3 CA = a - c; vec3 pc = p - c;
    vec3 n = cross(AB, CA);

    float q = sign(dot(cross(AB, n), pa)) + 
              sign(dot(cross(BC, n), pb)) + 
              sign(dot(cross(CA, n), pc));

    if (q >= 2.0f) 
        return sqrt(dot(n, pa) * dot(n, pa) / dot2(n));

    return sqrt(
        min(
            min(
                dot2(AB * clamp(dot(AB, pa) / dot2(AB), 0.0f, 1.0f) - pa),
                dot2(BC * clamp(dot(BC, pb) / dot2(BC), 0.0f, 1.0f) - pb)
            ), 
            dot2(CA * clamp(dot(CA, pc) / dot2(CA), 0.0f, 1.0f) - pc)
        )
    );
}

const int max_level = 8;

void main()
{
    //==========================================================================================================================================================
    // get the index of the triangle this invocation will work on 
    //==========================================================================================================================================================
    int head = atomicCounterIncrement(index_cntr);

    while (head < triangles)
    {
        //======================================================================================================================================================
        // get the indices and the vertices of the triangle
        //======================================================================================================================================================
        int base_index = 3 * head;

        int iA = imageLoad(index_buffer, base_index + 0);         
        int iB = imageLoad(index_buffer, base_index + 1);
        int iC = imageLoad(index_buffer, base_index + 2);

        vec3 vA = imageLoad(vertex_buffer, iA).xyz;
        vec3 vB = imageLoad(vertex_buffer, iB).xyz;
        vec3 vC = imageLoad(vertex_buffer, iC).xyz;

        //======================================================================================================================================================
        // calculate triangle diameter
        //======================================================================================================================================================
        vec3 BA = vB - vA; float lBA = length(BA);
        vec3 CB = vC - vB; float lCB = length(CB);
        vec3 AC = vA - vC; float lAC = length(AC);

        float diameter = max(max(lBA, lCB), lAC);


        int digits[max_level] = {0, 0, 0, 0, 0, 0, 0, 0};

        ivec3 Pi[max_level];
        Pi[0] = ivec3(128);

        int level = 1;

        while(level != 0)
        {
            if (digit[level] < 8)
            {

            }
            ivec3 Pi = 
            for(int )

        }



        //======================================================================================================================================================
        // done! proceed to next triangle
        //======================================================================================================================================================
        head = atomicCounterIncrement(index);
    }
}
