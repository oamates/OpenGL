#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

uniform sampler2D normal_ext_tex;
layout (r32f) uniform image2D normal_combined_image;

//==============================================================================================================================================================
// shader entry point
//==============================================================================================================================================================
void main()
{
    ivec2 Q = textureSize(normal_ext_tex, 0);
    ivec2 P = ivec2(gl_GlobalInvocationID.xy);
    if ((P.x >= Q.x) || (P.y >= Q.y)) return;

    vec2 texel_size = 1.0 / Q;
    vec2 uv0 = texel_size * (vec2(P) + 0.5);

    vec3 n = textureLod(normal_ext_tex, uv0, 0).rgb;

    imageStore(normal_combined_image, P, vec4(n, 1.0));
}
