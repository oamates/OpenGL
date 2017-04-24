#version 430 core

layout (local_size_x = 16, local_size_y = 16) in;

struct RAY
{
    ivec4  screen_origin;
    vec4   world_origin;
    vec4   world_direction;
};

layout (std430, binding = 0) buffer input_ray_buffer
{
    RAY    ray[];
} input_buffer;

layout (std430, binding = 1) buffer output_ray_buffer
{
    RAY ray[];
} output_buffer;

layout (binding = 0, offset = 0) uniform atomic_uint consume_counter;
layout (binding = 0, offset = 4) uniform atomic_uint append_counter;

layout (rgba32f, binding = 0) uniform image2D output_image;

void main(void)
{
    uint ray_index = atomicCounterIncrement(consume_counter);
    RAY input_ray = input_buffer.ray[ray_index];
    barrier();
    {
        imageStore(output_image, input_ray.screen_origin.xy, vec4(input_ray.world_direction.xyz, 1.0));
        ray_index = atomicCounterIncrement(append_counter);
        input_ray.world_direction.x += 0.01;
        output_buffer.ray[ray_index] = input_ray;
        ray_index = atomicCounterIncrement(append_counter);
        input_ray.world_direction.y += 0.01;
        output_buffer.ray[ray_index] = input_ray;
    }
}

