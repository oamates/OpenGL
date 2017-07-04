#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout (r32f, binding = 0) uniform image3D sdf;


void main()
{
    ivec3 id = ivec3(gl_GlobalInvocationID.xyz);
    imageStore(sdf, id, vec4(1.0f, 0.0, 0.0, 0.0));
}
