#version 430 core

layout (local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

//==============================================================================================================================================================
// Every workgroup will work on 8 x 8 pixel area
//==============================================================================================================================================================

uniform sampler2D input_tex;
layout (r32f) uniform image2D output_image;

uniform ivec2 image_size;
uniform vec2 texel_size;

//==============================================================================================================================================================
// shader entry point
//==============================================================================================================================================================
void main(void)
{
    ivec2 P = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv = texel_size * (vec2(P) + 0.5);

    vec3 c = texture(diffuse_tex, uv).rgb;
    vec3 hsv = rgb2hsv(c);
    hsv.y = 0.0f;
    vec3 rgb = hsv2rgb(hsv);

    float q = clamp(0.17 * dot(rgb, rgb), 0.0, 1.0);
    q = pow(q, 0.7);

    imageStore(shininess_image, P, vec4(q, 0.0, 0.0, 0.0));
}