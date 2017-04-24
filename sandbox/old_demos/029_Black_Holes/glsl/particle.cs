#version 430 core


layout (std430, binding = 0, shared) buffer shader_data
{
    vec4 attractor[64];
};

layout (local_size_x = 128) in;

layout (rgba32f, binding = 0) uniform imageBuffer velocity_buffer;
layout (rgba32f, binding = 1) uniform imageBuffer position_buffer;

uniform int attractor_count;
uniform float dt;
uniform float time;

void main(void)
{
    vec4 vel = imageLoad(velocity_buffer, int(gl_GlobalInvocationID.x));
    vec4 pos = imageLoad(position_buffer, int(gl_GlobalInvocationID.x));


    for (int i = 0; i < attractor_count; i++)
    {
        vec3 direction = attractor[i].xyz - pos.xyz;

        float hole_radius = attractor[i].w;
        float hole_mass = hole_radius * hole_radius * hole_radius;
        float l = length(direction);
        vec3 n = direction / l;

        vel.xyz += 15.0 * dt * hole_mass * n / l;
        if (l < hole_radius) vel.xyz = -reflect(vel.xyz, n);
    };

    pos.xyz += vel.xyz * dt;

    float l = length(pos);
    if (l > 250.0f)
    {
        float q = (l - 250.0f) * (l - 250.0f) * 0.0001f; 
        vel.xyz -= q * pos.xyz;
    };

    pos.w -= 0.044 * dt;
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