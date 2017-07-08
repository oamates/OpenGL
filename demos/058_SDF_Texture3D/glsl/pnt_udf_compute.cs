#version 430 core

layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

const int MAX_LEVEL = 10;

//==============================================================================================================================================================
// input :: level, the number of triangles to process and ...
//   image unit 0 :: index buffer of GL_TRIANGLES type
//   image unit 1 :: positions buffer, vec3 with w component ignored for now (format GL_RGB32F is not valid for GL_TEXTURE_BUFFER)
//==============================================================================================================================================================
uniform int level;
uniform int points;

layout (rgba32f, binding = 0) uniform imageBuffer position_buffer;

//==============================================================================================================================================================
// auxiliary buffer needed for search octree construction
// if the output texture is 2^{n} x 2^{n} x 2^{n}, then this buffer should be of
// the size 8 + 64 + ... + 2^{3n-3}
//==============================================================================================================================================================
layout (r32ui, binding = 1) uniform uimageBuffer octree;

//==============================================================================================================================================================
// atomic counter backed up by an atomic counter buffer for triangle separation among execution threads 
//==============================================================================================================================================================
layout (binding = 0, offset = 0) uniform atomic_uint triangle_index;

//==============================================================================================================================================================
// output 3d texture, of size 2^{n} x 2^{n} x 2^{n}, n = max_level
//==============================================================================================================================================================
layout (r32ui, binding = 3) uniform uimage3D udf_tex;

//==============================================================================================================================================================
// both octree and udf_tex have to be initialized with the value
// corresponding to cube diagonal size :: 2 * sqrt(3) * INTEGRAL_SCALE = 929887697 or greater value
//==============================================================================================================================================================

float dot2(vec3 v) 
    { return dot(v, v); }

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

const int p2 = 1 << level;
const int p2m1 = 1 << (level - 1);
const float inv_p2 = 1.0 / p2;

const float INTEGRAL_SCALE = 268435456.0;
const float INV_INT_SCALE = 1.0 / INTEGRAL_SCALE;
const float sqrt3 = 1.73205080757;
const float cube_diameter = 2.0 * sqrt3;

void main()
{
    //==========================================================================================================================================================
    // get the index of the triangle this invocation will work on 
    //==========================================================================================================================================================
    uint t = atomicCounterIncrement(triangle_index);

    while (t < triangles)
    {
        //======================================================================================================================================================
        // get the indices and the vertices of the triangle
        //======================================================================================================================================================
        int base_index = 3 * int(t);

        int iA = int(imageLoad(index_buffer, base_index + 0).x);        
        int iB = int(imageLoad(index_buffer, base_index + 1).x);
        int iC = int(imageLoad(index_buffer, base_index + 2).x);

        vec3 vA = imageLoad(vertex_buffer, iA).xyz;
        vec3 vB = imageLoad(vertex_buffer, iB).xyz;
        vec3 vC = imageLoad(vertex_buffer, iC).xyz;

        //======================================================================================================================================================
        // calculate triangle diameter
        //======================================================================================================================================================
        vec3 BA = vB - vA; float dBA = dot2(BA);
        vec3 CB = vC - vB; float dCB = dot2(CB);
        vec3 AC = vA - vC; float dAC = dot2(AC);

        float triangle_diameter = max(max(sqrt(dBA), sqrt(dCB)), sqrt(dAC));
        float inv_dBA = 1.0 / dBA;
        float inv_dCB = 1.0 / dCB;
        float inv_dAC = 1.0 / dAC;        

        vec3 normal = cross(BA, AC);
        float inv_area = 1.0 / length(normal);

        //======================================================================================================================================================
        // our position in the octree and corresponding index into octree buffer
        //======================================================================================================================================================
        int octree_digit[MAX_LEVEL];
        int node_index = 0;
        octree_digit[0] = 0;
        octree_digit[1] = 0;        

        //======================================================================================================================================================
        // the algorithm starts with jumping from level 0 to level 1 and recursively going down/up
        // when we come back to the level 0, distance octree will be traversed and updated
        //======================================================================================================================================================
        int l = 1;
        float scale = 0.5;

        vec3 node_position[MAX_LEVEL];
        node_position[0] = vec3(0.0);

        while(l != 0)
        {
            //==================================================================================================================================================
            // update the current position of the node and calculate the distance from the triangle to it
            //==================================================================================================================================================
            node_position[l] = node_position[l - 1] + scale * shift[octree_digit[l]];
            vec3 p = node_position[l];

            vec3 pA = p - vA;
            vec3 pB = p - vB;
            vec3 pC = p - vC;

            float q = sign(dot(cross(BA, normal), pA)) + sign(dot(cross(CB, normal), pB)) + sign(dot(cross(AC, normal), pC));

            float distance_to_node = (q >= 2.0) ? inv_area * abs(dot(normal, pA)) : 
                    sqrt(
                        min(
                            min(
                                dot2(clamp(dot(BA, pA) * inv_dBA, 0.0f, 1.0f) * BA - pA),
                                dot2(clamp(dot(CB, pB) * inv_dCB, 0.0f, 1.0f) * CB - pB)
                            ), 
                                dot2(clamp(dot(AC, pC) * inv_dAC, 0.0f, 1.0f) * AC - pC)
                        )
                    );

            uint idistance_to_node = uint(distance_to_node * INTEGRAL_SCALE);

            uint icurrent_distance = imageAtomicMin(octree, node_index, idistance_to_node);
            float current_distance = float(icurrent_distance) * INV_INT_SCALE;
            float node_diameter = (scale - inv_p2) * cube_diameter;


            //==================================================================================================================================================
            // compare the distance with the distance currently stored in octree
            //==================================================================================================================================================
            if (distance_to_node >= node_diameter + current_distance + triangle_diameter)
            {
                //==============================================================================================================================================
                // current_distance is small enough, the node can be skipped completely
                // either stay on the same level and take next digit or go up if the last digit (7) on the current level has been processed
                //==============================================================================================================================================
                while(octree_digit[l] == 7)
                {
                    scale += scale;
                    node_index = (node_index >> 3) - 1;
                    l--;
                }
                octree_digit[l]++;
                node_index++;
            }
            else
            {
                if (l == level - 1)
                {
                    //==========================================================================================================================================
                    // we came to 8 octree leafs, compute the 8 distances and do atomicMin
                    //==========================================================================================================================================
                    vec3 leaf_node = node_position[level - 1];
                    for(int v = 0; v < 8; ++v)
                    {
                        vec3 leaf_position = leaf_node + inv_p2 * shift[v];
                        ivec3 uvw = ivec3(floor(p2m1 + p2m1 * leaf_position));

                        vec3 pA = leaf_position - vA;
                        vec3 pB = leaf_position - vB;
                        vec3 pC = leaf_position - vC;

                        float q = sign(dot(cross(BA, normal), pA)) + sign(dot(cross(CB, normal), pB)) + sign(dot(cross(AC, normal), pC));
                        float distance_to_leaf = (q >= 2.0f) ? inv_area * abs(dot(normal, pA)) : 
                            sqrt(
                                min(
                                    min(
                                        dot2(clamp(dot(BA, pA) * inv_dBA, 0.0, 1.0) * BA - pA),
                                        dot2(clamp(dot(CB, pB) * inv_dCB, 0.0, 1.0) * CB - pB)
                                    ), 
                                        dot2(clamp(dot(AC, pC) * inv_dAC, 0.0, 1.0) * AC - pC)
                                )
                            );

                        uint idistance_to_leaf = uint(distance_to_leaf * INTEGRAL_SCALE);
                        imageAtomicMin(udf_tex, uvw, idistance_to_leaf);
                    }

                    //==========================================================================================================================================
                    // stay on the same level and take next digit or go up if the last digit (=7) on the current level has been processed
                    //==========================================================================================================================================
                    while(octree_digit[l] == 7)
                    {
                        scale += scale;
                        node_index = (node_index >> 3) - 1;
                        l--;
                    }
                    octree_digit[l]++;
                    node_index++;
                }
                else
                {
                    //==========================================================================================================================================
                    // go down
                    //==========================================================================================================================================
                    l++;
                    octree_digit[l] = 0;
                    node_index = (node_index + 1) << 3;
                    scale *= 0.5;
                }
            }
        }

        //======================================================================================================================================================
        // done ... proceed to next triangle
        //======================================================================================================================================================
        t = atomicCounterIncrement(triangle_index);
    }
}