

#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout (r32ui, binding = 0) uniform uimage3D udf;

uniform uint value;

void main()
{
    ivec3 id = ivec3(gl_GlobalInvocationID.xyz);
    imageStore(udf, id, uvec4(value, 0.0, 0.0, 0.0));
}
