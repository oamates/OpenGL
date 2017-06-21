#version 430 core

layout (local_size_x = 128) in;

layout (rgba32f, binding = 0) uniform imageBuffer position_buffer;
layout (rgba32f, binding = 1) uniform imageBuffer normal_buffer;

uniform float dt;
uniform float time;

//==============================================================================================================================================================
// canyon distance function
//==============================================================================================================================================================
vec3 tri(in vec3 x)
{
    vec3 q = abs(fract(x) - 0.5f);
    return q;
}

float sdf(vec3 p)
{   
    p *= 8.75; 
    vec3 op = tri(1.1f * p + tri(1.1f * p.zxy));
    p += (op - 0.25) * 0.3;
    p = cos(0.444f * p + sin(1.112f * p.zxy));
    float canyon = (length(p) - 1.05) * 0.95;
    return canyon;
}

//==============================================================================================================================================================
// sdf gradient :: tetrahedral evaluation
//==============================================================================================================================================================
vec3 grad(in vec3 p)
{
    vec2 e = vec2(0.0125, -0.0125);
    return normalize(e.xyy * sdf(p + e.xyy) + e.yyx * sdf(p + e.yyx) + e.yxy * sdf(p + e.yxy) + e.xxx * sdf(p + e.xxx));
}

void main(void)
{
    int id = int(gl_GlobalInvocationID.x);
    vec4 p4 = imageLoad(position_buffer, id);
    vec4 n4 = imageLoad(normal_buffer, id);

    vec3 position = vec3(p4);
    float field_value = p4.w;
    vec3 gradient = n4.xyz;

    vec3 velocity = -0.0625f * field_value * gradient;
    position = position + dt * velocity;
    field_value = sdf(position);

    vec3 normal_ws = grad(position);
    n4 = vec4(normal_ws, 0.0);
    p4 = vec4(position, field_value);

    imageStore(position_buffer, id, p4);
    imageStore(normal_buffer, id, n4);
}