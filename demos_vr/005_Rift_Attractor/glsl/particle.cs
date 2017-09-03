#version 430 core

layout (std140, binding = 0) uniform attractor_block
{
    vec4 attractor[2];
};

layout (local_size_x = 128) in;

layout (rgba32f, binding = 0) uniform imageBuffer velocity_buffer;
layout (rgba32f, binding = 1) uniform imageBuffer position_buffer;

uniform float dt;
uniform float time;

void main(void)
{
    vec4 vel = imageLoad(velocity_buffer, int(gl_GlobalInvocationID.x));
    vec4 pos = imageLoad(position_buffer, int(gl_GlobalInvocationID.x));

    pos.xyz += vel.xyz * dt;

    for (int i = 0; i < 2; i++)
    {
        vec3 direction = attractor[i].xyz - pos.xyz;
        float l = length(direction);
        vel.xyz += dt * dt * attractor[i].w * normalize(direction) / (0.00001 + l * l);
    };

    float l = length(pos);
    if (l > 250.0f)
    {
        float q = (l - 250.0f) * (l - 250.0f) * 0.0001f; 
        vel.xyz -= q * pos.xyz;
    };

    pos.w -= 0.0044 * dt;
    if (pos.w <= 0.0)
    {
        if (pos.z > 0)
            pos.xyz =  vec3(100.0 * cos(0.53 * time), 100.0 * sin(0.53 * time), 0.0f) + 0.015 * pos.xyz;
        else
            pos.xyz = -vec3(100.0 * cos(0.53 * time), 100.0 * sin(0.53 * time), 0.0f) + 0.015 * pos.xyz;
        vel.xyz *= 0.05;
        pos.w += 1.0f;
    };

    imageStore(position_buffer, int(gl_GlobalInvocationID.x), pos);
    imageStore(velocity_buffer, int(gl_GlobalInvocationID.x), vel);
}