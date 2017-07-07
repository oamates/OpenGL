#version 430 core

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

const int n = 8;
const int p2 = 1 << n;
const int p2m1 = 1 << (n - 1);
const float inv_p2 = 1.0 / p2;
const float inv_p2m1 = 1.0 / p2m1;

//==============================================================================================================================================================
// input :: the number of triangles to process and ...
//   image unit 0 :: index buffer of GL_TRIANGLES type
//   image unit 1 :: positions buffer, vec3 with w component ignored for now
//==============================================================================================================================================================
uniform uint triangles;

layout (r32ui, binding = 0) uniform uimageBuffer index_buffer;
layout (rgba32f, binding = 1) uniform imageBuffer vertex_buffer;

//==============================================================================================================================================================
// auxiliary buffer needed for search octree construction
// if the output texture is 2^{n} x 2^{n} x 2^{n}, then this buffer should be of
// the size 8 + 64 + ... + 2^{3n-3}
// and atomic counter backed up by an atomic counter buffer for triangle separation among execution threads 
//==============================================================================================================================================================
layout (r32ui, binding = 2) uniform uimageBuffer octree;
layout (binding = 0, offset = 0) uniform atomic_uint triangle_index;

//==============================================================================================================================================================
// output 3d texture, of size 
//==============================================================================================================================================================
layout (r32ui, binding = 0) uniform uimage3D udf_tex;

//==============================================================================================================================================================
// unsigned distance from point to triangle function
// since it is going to be used millions of time it is inlined into the main body
//==============================================================================================================================================================

float dot2(vec3 v) 
    { return dot(v, v); }

/*
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
*/

const vec3 shift[8] = vec3[]
(
    vec3(-1.0, -1.0, -1.0),
    vec3( 1.0, -1.0, -1.0),
    vec3(-1.0,  1.0, -1.0),
    vec3( 1.0,  1.0, -1.0),
    vec3(-1.0, -1.0,  1.0),
    vec3( 1.0, -1.0,  1.0),
    vec3(-1.0,  1.0,  1.0),
    vec3( 1.0,  1.0,  1.0)
);

const float INTEGRAL_SCALE = 268435456.0;
const float INV_INT_SCALE = 1.0 / INTEGRAL_SCALE;

const float sqrt3 = 1.73205080757;

float node_diameter[n] = float []
(
    (2.000000 - inv_p2m1) * sqrt3,
    (1.000000 - inv_p2m1) * sqrt3, 
    (0.500000 - inv_p2m1) * sqrt3, 
    (0.250000 - inv_p2m1) * sqrt3,
    (0.125000 - inv_p2m1) * sqrt3, 
    (0.062500 - inv_p2m1) * sqrt3, 
    (0.031250 - inv_p2m1) * sqrt3, 
    (0.015625 - inv_p2m1) * sqrt3
);



void main()
{
    //==========================================================================================================================================================
    // get the index of the triangle this invocation will work on 
    //==========================================================================================================================================================
    uint triangle = atomicCounterIncrement(triangle_index);

    while (triangle < triangles)
    {
        //======================================================================================================================================================
        // get the indices and the vertices of the triangle
        //======================================================================================================================================================
        int base_index = 3 * int(triangle);

        uint iA = imageLoad(index_buffer, base_index + 0).x;        
        uint iB = imageLoad(index_buffer, base_index + 1).x;
        uint iC = imageLoad(index_buffer, base_index + 2).x;

        vec3 vA = imageLoad(vertex_buffer, int(iA)).xyz;
        vec3 vB = imageLoad(vertex_buffer, int(iB)).xyz;
        vec3 vC = imageLoad(vertex_buffer, int(iC)).xyz;

        //======================================================================================================================================================
        // calculate triangle diameter
        //======================================================================================================================================================

        vec3 BA = vB - vA; float dBA = dot2(BA);
        vec3 CB = vC - vB; float dCB = dot2(CB);
        vec3 AC = vA - vC; float dAC = dot2(AC);

        float triangle_diameter = max(max(sqrt(dBA), sqrt(dCB)), sqrt(dAC));

        vec3 normal = cross(BA, AC);
        float inv_area = length(normal);

        //======================================================================================================================================================
        // our position in the octree and corresponding index into octree buffer
        //======================================================================================================================================================
        int octree_digit[n] = {0, 0, 0, 0, 0, 0, 0, 0};
        int node_index = 0;

        //======================================================================================================================================================
        // the algorithm starts with jumping from level 0 to level 1 and recursively going down/up
        // when we come back to the level 0, distance octree will be traversed and updated
        //======================================================================================================================================================
        int level = 1;
        float scale = 0.5;

        vec3 node_position[n];
        node_position[0] = vec3(0.0);

        while(level != 0)
        {
            //==================================================================================================================================================
            // update the current position of the node and calculate the distance from the triangle to it
            //==================================================================================================================================================
            node_position[level] = node_position[level - 1] + scale * shift[octree_digit[level]];
            vec3 p = node_position[level];

            vec3 pA = p - vA;
            vec3 pB = p - vB;
            vec3 pC = p - vC;

            float q = sign(dot(cross(BA, normal), pA)) + sign(dot(cross(CB, normal), pB)) + sign(dot(cross(AC, normal), pC));

            float distance_to_node = (q >= 2.0f) ? inv_area * abs(dot(normal, pA)) : 
                    sqrt(
                        min(
                            min(
                                dot2(BA * clamp(dot(BA, pA) / dBA, 0.0f, 1.0f) - pA),
                                dot2(CB * clamp(dot(CB, pB) / dCB, 0.0f, 1.0f) - pB)
                            ), 
                                dot2(AC * clamp(dot(AC, pC) / dAC, 0.0f, 1.0f) - pC)
                        )
                    );

            uint idistance_to_node = uint(distance_to_node * INTEGRAL_SCALE);

            uint icurrent_distance = imageAtomicMin(octree, node_index, idistance_to_node);
            float current_distance = float(icurrent_distance) * INV_INT_SCALE;

            //==================================================================================================================================================
            // compare the distance with the distance currently stored in octree
            //==================================================================================================================================================
            if (distance_to_node >= node_diameter[level] + current_distance + triangle_diameter)
            {
                //==================================================================================================================================================
                // current_distance is small enough, the node can be skipped completely
                // either stay on the same level or go up if digit[level] == 7
                //==================================================================================================================================================
                while(octree_digit[level] == 7)
                {
                    scale += scale;
                    node_index = (node_index >> 3) - 1;
                    level--;
                }
                octree_digit[level]++;
                node_index++;
            }
            else
            {
                if (level == n - 1)
                {
                    //==============================================================================================================================================
                    // we came to 8 octree leafs, compute the 8 distances and do atomicMin
                    //==============================================================================================================================================
                    vec3 leaf_node = node_position[n - 1];
                    for(int v = 0; v < 8; ++v)
                    {
                        vec3 leaf_position = leaf_node + inv_p2 * shift[v];
                        ivec3 uvw = ivec3(floor(128.0 + 128.0 * leaf_position));

                        vec3 pA = leaf_position - vA;
                        vec3 pB = leaf_position - vB;
                        vec3 pC = leaf_position - vC;

                        float q = sign(dot(cross(BA, normal), pA)) + sign(dot(cross(CB, normal), pB)) + sign(dot(cross(AC, normal), pC));
                        float distance_to_leaf = (q >= 2.0f) ? inv_area * abs(dot(normal, pA)) : 
                            sqrt(
                                min(
                                    min(
                                        dot2(BA * clamp(dBA / dot2(BA), 0.0f, 1.0f) - pA),
                                        dot2(CB * clamp(dCB / dot2(CB), 0.0f, 1.0f) - pB)
                                    ), 
                                        dot2(AC * clamp(dAC / dot2(AC), 0.0f, 1.0f) - pC)
                                )
                            );

                        uint idistance_to_leaf = uint(distance_to_leaf * INTEGRAL_SCALE);

                        imageAtomicMin(udf_tex, uvw, idistance_to_leaf);

                    }

                    //==============================================================================================================================================
                    // come back one/more levels up in the octree
                    //==============================================================================================================================================
                    while(octree_digit[level] == 7)
                    {
                        scale += scale;
                        node_index = (node_index >> 3) - 1;
                        level--;
                    }
                    octree_digit[level]++;
                    node_index++;
                }
                else
                {
                    //==============================================================================================================================================
                    // go down
                    //==============================================================================================================================================
                    level++;
                    octree_digit[level] = 0;
                    node_index = (node_index + 1) << 3;
                    scale *= 0.5;
                }
            }
        }

        //======================================================================================================================================================
        // done ... proceed to next triangle
        //======================================================================================================================================================
        triangle = atomicCounterIncrement(triangle_index);
    }
}
