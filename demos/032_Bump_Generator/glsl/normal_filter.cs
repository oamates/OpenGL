#version 430 core

//==============================================================================================================================================================
// Every workgroup will work on 8 x 8 pixel area
//==============================================================================================================================================================
layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout (r32f, binding = 1) uniform image2D luminosity_image;
layout (rgba32f, binding = 2) uniform image2D normal_image;

const float amplitude = 4.0f;

float luma(ivec2 P)
{
    return imageLoad(luminosity_image, P).r;
}

//==============================================================================================================================================================
// shader entry point
//==============================================================================================================================================================
void main()
{
    ivec2 B = ivec2(gl_NumWorkGroups.xy * gl_WorkGroupSize.xy);
    ivec2 P = ivec2(gl_GlobalInvocationID.xy);
    ivec2 Pmin = max(P - 1, -P);
    ivec2 Pmax = min(P + 1, 2 * B - P - 2);

    float bl = luma(ivec2(Pmin.x, Pmin.y));             // bottom left
    float bc = luma(ivec2(P.x,    Pmin.y));             // bottom center
    float br = luma(ivec2(Pmax.x, Pmin.y));             // bottom right
    float cl = luma(ivec2(Pmin.x, P.y   ));             // center left
    float cr = luma(ivec2(Pmax.x, P.y   ));             // center right
    float tl = luma(ivec2(Pmin.x, Pmax.y));             // top left
    float tc = luma(ivec2(P.x,    Pmax.y));             // top center
    float tr = luma(ivec2(Pmax.x, Pmax.y));             // top right

    /* Sobel filter */
    /*
    float dL_dx = (br + 2.0 * cr + tr) - (bl + 2.0 * cl + tl);
    float dL_dy = (tl + 2.0 * tc + tr) - (bl + 2.0 * bc + br);
    dL_dx *= 0.25;
    dL_dy *= 0.25;
    */

    /* Scharr filter */
    float dL_dx = (3.0 * br + 10.0 * cr + 3.0 * tr) - (3.0 * bl + 10.0 * cl + 3.0 * tl);
    float dL_dy = (3.0 * tl + 10.0 * tc + 3.0 * tr) - (3.0 * bl + 10.0 * bc + 3.0 * br);
    dL_dx *= 0.0625;
    dL_dy *= 0.0625;

    vec3 n = normalize(vec3(amplitude * vec2(dL_dx, dL_dy), 1.0));
    imageStore(normal_image, P, vec4(0.5 + 0.5 * n, 1.0));
}