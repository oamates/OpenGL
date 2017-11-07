#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

uniform sampler2D normal_tex;
layout (r32f) uniform image2D normal_ext_image;
uniform int tex_level;

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
    vec3 n = textureLod(normal_tex, uv0, lod).rgb;


    imageStore(normal_ext_image, P, vec4(n, 1.0));
}

/*
subroutine(filterModeType) vec4 mode_normal_expansion_filter()
{
    vec3 filt  = vec3(0);
    float wtotal = 0.0;
    int radius   = int(gui_filter_radius);

    for(int i = -radius; i <= radius; i++)
    {
        for(int j = -radius; j <= radius; j++)
        {
            vec2 coords = vec2(v2QuadCoords.xy + vec2(i,j) * dxy);
            vec3 normal = normalize(2*texture(layerA,coords).xyz-1);

            float w = mix(length(normal.xy), 1 / (20 * gaussian(vec2(i, j), gui_filter_radius) * length(normal.xy) + 1), gui_normal_flatting + 0.001);
            wtotal += w;
            filt += normal * w;
        }
    }

    filt /= (wtotal);                                                               // normalization
    return vec4(0.5 * normalize(filt) + 0.5, 1.0f);
}
*/