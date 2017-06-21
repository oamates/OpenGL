#version 430 core

layout (local_size_x = 128) in;

layout (rgba32f, binding = 0) uniform imageBuffer position_buffer;

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
    vec3 op = tri(1.1f * p + tri(1.1f * p.zxy));
    float ground = p.z + 3.5 + dot(op, vec3(0.067));
    p += (op - 0.25) * 0.3;
    p = cos(0.444f * p + sin(1.112f * p.zxy));
    float canyon = (length(p) - 1.05) * 0.95;
    return min(ground, canyon);
}

//==============================================================================================================================================================
// sdf gradient :: tetrahedral evaluation
//==============================================================================================================================================================
vec3 grad(in vec3 p)
{
    vec2 e = vec2(0.0125, -0.0125);
    return normalize(e.xyy * sdf(p + e.xyy) + e.yyx * sdf(p + e.yyx) + e.yxy * sdf(p + e.yxy) + e.xxx * sdf(p + e.xxx));
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main(void)
{
    vec4 p4 = imageLoad(position_buffer, int(gl_GlobalInvocationID.x));

    vec3 position = vec3(p4);
    float intensity = p4.w;

    float field_value = sdf(position);
    vec3 gradient = grad(position);
    vec3 velocity = -0.5f * field_value * gradient;
    position = position + dt * velocity;

    p4 = vec4(position, intensity);

    imageStore(position_buffer, int(gl_GlobalInvocationID.x), p4);
}