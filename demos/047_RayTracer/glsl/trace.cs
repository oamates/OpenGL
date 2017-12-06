#version 430 core

layout (local_size_x = 8, local_size_y = 8) in;

//==============================================================================================================================================================
// ray SSBO
//==============================================================================================================================================================
struct ray_t
{
    ivec4 screen;
    vec4 origin;
    vec4 direction;
};

layout (std430, binding = 0) buffer input_ray_buffer
{
    ray_t rays[];
} input_buffer;

layout (std430, binding = 1) buffer output_ray_buffer
{
    ray_t rays[];
} output_buffer;

//==============================================================================================================================================================
// sphere objects uniform buffer
//==============================================================================================================================================================
layout (std430, binding = 0) uniform geometry_buffer
{
    ray_t rays[];
} output_buffer;

//==============================================================================================================================================================
// atomic counters
//==============================================================================================================================================================
layout (binding = 0, offset = 0) uniform atomic_uint consume_counter;
layout (binding = 0, offset = 4) uniform atomic_uint append_counter;

//==============================================================================================================================================================
// output image
//==============================================================================================================================================================
layout (rgba32f, binding = 0) uniform image2D output_image;

void main()
{
    uint ray_index = atomicCounterIncrement(consume_counter);
    ray_t input_ray = input_buffer.rays[ray_index];
    barrier();

    imageStore(output_image, input_ray.screen.xy, vec4(abs(input_ray.direction.xyz), 1.0));

    ray_index = atomicCounterIncrement(append_counter);
    input_ray.direction.x += 0.01;
    output_buffer.rays[ray_index] = input_ray;

    ray_index = atomicCounterIncrement(append_counter);
    input_ray.direction.y += 0.01;
    output_buffer.rays[ray_index] = input_ray;
}

