#version 430 core

layout (local_size_x = 16, local_size_y = 16) in;

struct RAY
{
    ivec4  screen_origin;
    vec4   world_origin;
    vec4   world_direction;
};

layout (std430, binding = 0) buffer ray_buffer
{
    RAY    ray[];
};

RAY initialize_ray()
{
    RAY r;
    r.screen_origin.xy = ivec2(gl_GlobalInvocationID.xy);
    r.world_origin.xyz = vec3(0.0, 0.0, 0.0);
    r.world_direction.xyz = normalize(vec3(vec2(gl_LocalInvocationID.xy), 100.0f));
    return r;
}

void main(void)
{
    ray[gl_LocalInvocationID.y * 16 + gl_LocalInvocationID.x] = initialize_ray();
}

