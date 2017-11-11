#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

const int MAX_LOD = 5;

uniform sampler2D normal_ext_tex;
layout (r32f) uniform image2D normal_combined_image;

uniform float lod_intensity[MAX_LOD];

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

    vec3 n = vec3(0.0);
    float magnitude = 1.0f;

    for(int l = 0; l < MAX_LOD; ++l)
    {
        float lod = l;
        float intensity = lod_intensity[l];
        vec3 n_lod = 2.0 * textureLod(normal_ext_tex, uv0, lod).rgb - 1.0;
        n += magnitude * vec3(intensity, intensity, 1.0f) * normalize(n_lod);
    }

    n = normalize(n);
    imageStore(normal_combined_image, P, vec4(0.5 + 0.5 * n, 1.0));
}
