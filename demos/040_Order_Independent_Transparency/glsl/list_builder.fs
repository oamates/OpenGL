#version 430 core

layout (early_fragment_tests) in;

in vec3 position_ws;
in vec3 normal_ws;
in vec3 view;
in vec3 light;
in vec2 uv;
in float alpha;

uniform sampler2D tb_tex;

layout (binding = 0, r32ui) uniform uimage2D head_pointer_image;
layout (binding = 1, rgba32ui) uniform writeonly uimageBuffer list_buffer;
layout (binding = 0, offset = 0) uniform atomic_uint list_counter;

vec3 tex2d(vec2 uv)
{
    return texture(tb_tex, uv).rgb;
}

vec3 tex3d(in vec3 p, in vec3 n)
{
    p *= 1.4875;
    vec3 w = max(abs(n) - 0.317f, 0.0f);
    w /= dot(w, vec3(1.0f));
    mat3 rgb_samples = mat3(tex2d(p.yz), tex2d(p.zx), tex2d(p.xy));
    return rgb_samples * w;
}


void main(void)
{
    uint head = atomicCounterIncrement(list_counter);
    uint old_head = imageAtomicExchange(head_pointer_image, ivec2(gl_FragCoord.xy), uint(head));

    float distance = length(light);
    vec3 l = light / distance;                                      
    vec3 n = normal_ws;
    vec3 e = normalize(view);                                                         
    vec3 r = reflect(l, n);

    float cos_theta = 0.5f + 0.5f * dot(n, l);
    float cos_alpha = (0.5f + 0.5f * dot(e, r)) * cos_theta;

    float strength_factor = 1.0 / (1.0 + 0.001 * distance);

    vec3 diffuse_color = tex3d(position_ws, normalize(n));
    vec4 modulator = vec4(diffuse_color * cos_theta * strength_factor, alpha);
    vec4 additive_component = vec4(diffuse_color * vec3(0.71, 0.61, 0.44) * vec3(pow(cos_alpha, 32.0)) * strength_factor, 0.5f);

    uvec4 item = uvec4(old_head, packUnorm4x8(modulator), packUnorm4x8(additive_component), floatBitsToUint(gl_FragCoord.z));

    imageStore(list_buffer, int(head), item);
}
