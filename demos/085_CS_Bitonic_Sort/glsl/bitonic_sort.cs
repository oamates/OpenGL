#version 430 core

layout (local_size_x = 128) in;
layout (r32i, binding = 0) uniform iimageBuffer theArray;

uniform int stage;
uniform int pass;

void main()
{
    int id = int(gl_GlobalInvocationID.x);
    
    int pairDistance = 1 << (stage - pass);
    int blockWidth   = 2 * pairDistance;

    int leftId = (id % pairDistance) + (id / pairDistance) * blockWidth;
    int rightId = leftId + pairDistance;
    
    int leftElement  = imageLoad(theArray, leftId).x;
    int rightElement = imageLoad(theArray, rightId).x;

    int sameDirectionBlockWidth = 1 << stage;
    
    bool increasing = ((id / sameDirectionBlockWidth) % 2 == 1) ? false : true;

    int greater;
    int lesser;

    if(leftElement > rightElement)
    {
        greater = leftElement;
        lesser  = rightElement;
    }
    else
    {
        greater = rightElement;
        lesser  = leftElement;
    }
    
    if(increasing)
    {
        imageStore(theArray, leftId, ivec4(lesser, 0, 0, 0));
        imageStore(theArray, rightId, ivec4(greater, 0, 0, 0));
    }
    else
    {
        imageStore(theArray, leftId, ivec4(greater, 0, 0, 0));
        imageStore(theArray, rightId, ivec4(lesser, 0, 0, 0));
    }
}
