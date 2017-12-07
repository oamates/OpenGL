#version 430 core

layout (local_size_x = 8, local_size_y = 8) in;

uniform vec2 inv_res;                                                       // inverse screen dimensions
uniform vec2 focal_scale;                                                   // camera focal scale
uniform mat3 camera_matrix;                                                 // camera matrix
uniform vec3 camera_ws;                                                     // camera world-space position

struct ray_t
{
    ivec4 screen;
    vec4 origin;
    vec4 direction;
};

layout (std430, binding = 0) buffer ray_buffer
{
    ray_t rays[];
};

void main()
{
    vec2 uv = inv_res * (vec2(gl_GlobalInvocationID.xy) + 0.5f);            // pixel half-integral coordinates
    vec2 ndc = 2.0f * uv - 1.0f;                                            // normalized device coordinates
    vec3 z_uv = vec3(focal_scale * ndc, -1.0);
    vec3 v = normalize(camera_matrix * z_uv);

    ray_t ray;

    ray.screen.xy = ivec2(gl_GlobalInvocationID.xy);
    ray.origin = vec4(camera_ws, 1.0f);
    ray.direction = vec4(v, 0.0f);

    rays[gl_GlobalInvocationID.y * gl_WorkGroupSize.x * gl_NumWorkGroups.x + gl_GlobalInvocationID.x] = ray;
}

