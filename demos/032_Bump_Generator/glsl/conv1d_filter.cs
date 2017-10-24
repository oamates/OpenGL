#version 430 core

//==============================================================================================================================================================
// Every workgroup will work on 8 x 8 pixel area
//==============================================================================================================================================================
layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

const int MAX_KERNEL_SIZE = 16;
uniform int kernel_size;

uniform float radius[MAX_KERNEL_SIZE];
uniform float weight[MAX_KERNEL_SIZE];

uniform vec2 texel_size;
uniform vec2 axis;

uniform sampler2D input_tex;
layout (rgba32f) uniform image2D conv_image;

void main(void)
{
    ivec2 P = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv = texel_size * (vec2(P) + 0.5);

    vec3 acc = vec3(0.0f);
    for (int i = 0; i < kernel_size; ++i)
    {
        vec2 duv = radius[i] * axis;
        acc += weight[i] * texture(input_tex, uv + duv).rgb;
        acc += weight[i] * texture(input_tex, uv - duv).rgb; 
    }

    imageStore(conv_image, P, vec4(acc, 1.0));
}