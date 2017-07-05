#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout (r32f, binding = 0) uniform image3D sdf;

layout (r32ui, binding = 1) uniform uimage3D udf_1;
layout (r32ui, binding = 2) uniform uimage3D udf_2;

void main()
{
    ivec3 id = ivec3(gl_GlobalInvocationID.xyz);

    uint q1 = imageLoad(udf_1, id).x;
    uint q2 = imageLoad(udf_2, id).x;

    float z = float(q1) - float(q2);

    imageStore(sdf, id, vec4(z, 0.0, 0.0, 0.0));
}
