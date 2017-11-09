#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

uniform sampler2D normal_tex;
layout (r32f) uniform image2D normal_ext_image;
uniform int tex_level;
uniform float extension_radius;

//==============================================================================================================================================================
// shader entry point
//==============================================================================================================================================================
void main()
{
    ivec2 Q = textureSize(normal_tex, tex_level);
    ivec2 P = ivec2(gl_GlobalInvocationID.xy);
    if ((P.x >= Q.x) || (P.y >= Q.y)) return;

    vec2 texel_size = 1.0 / Q;
    vec2 uv0 = texel_size * (vec2(P) + 0.5);

    float lod = tex_level;
    vec3 n = vec3(0.0);
    const int radius = 3;

    for(int i = -radius; i <= radius; i++)
    {
        for(int j = -radius; j <= radius; j++)
        {
            vec2 p = vec2(i, j);
            vec2 uv = uv0 + texel_size * vec2(i, j);
            vec3 normal = 2.0 * texture(normal_tex, uv, lod).rgb - 1.0;
            float l = pow(length(normal.xy), 2.4);

            n += l * exp2(-length(p)) * normal;
        }
    }

    n = normalize(n);
    imageStore(normal_ext_image, P, vec4(0.5 + 0.5 * n, 1.0));
}