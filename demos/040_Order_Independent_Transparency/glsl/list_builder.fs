#version 430 core

layout (early_fragment_tests) in;

in vec3 position_ws;
in vec3 normal_direction;
in vec3 view_direction;
in vec3 light_direction;
in vec2 texture_coords;
in float alpha;

layout (binding = 0, r32ui) uniform uimage2D head_pointer_image;
layout (binding = 1, rgba32ui) uniform writeonly uimageBuffer list_buffer;
layout (binding = 0, offset = 0) uniform atomic_uint list_counter;

uniform float time;




float permute(float x) 
{
    return mod((34.0f * x + 1.0f) * x, 289.0f); 
}

vec2 grad2(vec2 p, float rot)                                               // Gradient mapping with an extra rotation.
{                                                                           // Map from a line to a diamond such that a shift maps to a rotation.
    const float K = 0.0243902439f;                                          // 1/41
    float u = permute(permute(p.x) + p.y) * K + rot;                        // Rotate by shift
    u = 4.0f * fract(u) - 2.0f;
    return vec2(abs(u) - 1.0f, abs(abs(u + 1.0f) - 2.0f) - 1.0f);
}

float srdnoise(in vec2 P, in float rot, out vec2 grad)
{
    const float F2 = 0.366025403f;                                          // Helper constants
    const float G2 = 0.211324865f;
    const float K = 0.0243902439f;                                          // 1/41
    vec2 Ps = P + dot(P, vec2(F2));                                         // Transform input point to the skewed simplex grid
    vec2 Pi = floor(Ps);                                                    // Round down to simplex origin
    vec2 P0 = Pi - dot(Pi, vec2(G2));                                       // Transform simplex origin back to (x,y) system
    vec2 v0 = P - P0;                                                       // Find (x,y) offsets from simplex origin to first corner
    vec2 i1 = (v0.x > v0.y) ? vec2(1.0f, 0.0f) : vec2 (0.0f, 1.0f);         // Pick (+x, +y) or (+y, +x) increment sequence
    vec2 v1 = v0 - i1 + G2;                                                 // Determine the offsets for the other two corners
    vec2 v2 = v0 - 1.0f + 2.0f * G2;
    Pi = mod(Pi, 289.0f);                                                   // Wrap coordinates at 289 to avoid float precision problems
    vec3 t = max(0.5f - vec3(dot(v0,v0), dot(v1,v1), dot(v2,v2)), 0.0f);    // Calculate the circularly symmetric part of each noise wiggle
    vec3 t2 = t*t;
    vec3 t4 = t2*t2;
    vec2 g0 = grad2(Pi, rot);                                               // Calculate the gradients for the three corners
    vec2 g1 = grad2(Pi + i1, rot);
    vec2 g2 = grad2(Pi + 1.0f, rot);
    vec3 gv = vec3(dot(g0, v0), dot(g1, v1), dot(g2, v2));                  // Compute noise contributions from each corner
    vec3 n = t4 * gv;                                                       // Circular kernel times linear ramp
    vec3 temp = t2 * t * gv;                                                // Compute partial derivatives in x and y
    vec3 gradx = temp * vec3(v0.x, v1.x, v2.x);
    vec3 grady = temp * vec3(v0.y, v1.y, v2.y);
    grad.x = -8.0f * (gradx.x + gradx.y + gradx.z);
    grad.y = -8.0f * (grady.x + grady.y + grady.z);
    grad.x += dot(t4, vec3(g0.x, g1.x, g2.x));
    grad.y += dot(t4, vec3(g0.y, g1.y, g2.y));
    grad *= 40.0f;
    return 40.0f * (n.x + n.y + n.z);                                       // Add contributions from the three corners and return
}

vec3 flow_noise_color(vec3 position)
{
    vec2 g1, g2;
    vec2 p = 10.0f * position.xy;
    float n1 = srdnoise(p * 0.5f, 0.2f * time, g1);
    float n2 = srdnoise(p * 2.0f + g1 * 0.5f, 0.51f * time, g2);
    float n3 = srdnoise(p * 4.0f + g1 * 0.5f + g2 * 0.25f, 0.77f * time, g2);
    return vec3(0.4f, 0.5f, 0.6f) + vec3(n1 + 0.75f * n2 + 0.5f * n3);
}













void main(void)
{
    uint index = atomicCounterIncrement(list_counter);
    uint old_head = imageAtomicExchange(head_pointer_image, ivec2(gl_FragCoord.xy), uint(index));

    float light_distance = length(light_direction);
    vec3 l = light_direction / light_distance;                                      
    vec3 n = normal_direction;
    vec3 e = normalize(view_direction);                                                         
    vec3 r = reflect(l, n);

    float cos_theta = 0.5f + 0.5f * dot(n, l);
    float cos_alpha = (0.5f + 0.5f * dot(e, r)) * cos_theta;

    float strength_factor = 1.0 / (1.0 + 0.00001 * light_distance);

    vec3 diffuse_color = flow_noise_color(vec3(texture_coords, 1.0)) * vec3(1.3, 1.3, 0.5);
    vec4 modulator = vec4(diffuse_color * cos_theta * strength_factor, alpha);
    vec4 additive_component = vec4(diffuse_color * vec3(0.71, 0.44, 0.44) * vec3(pow(cos_alpha, 16.0)) * strength_factor, 0.5f);

    uvec4 item = uvec4(
        old_head,
        packUnorm4x8(modulator),
        floatBitsToUint(gl_FragCoord.z),
        packUnorm4x8(additive_component));

    imageStore(list_buffer, int(index), item);
}
