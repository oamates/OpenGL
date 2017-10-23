#version 430 core

//==============================================================================================================================================================
// Every workgroup will work on 8 x 8 pixel area
//==============================================================================================================================================================
layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout (rgba32f, binding = 2) uniform image2D normal_image;
layout (r32f, binding = 3) uniform image2D displacement_image;
layout (r32f, binding = 6) uniform image2D aux_image;

uniform int n;

float normal_x(ivec2 P)
{
    return imageLoad(normal_image, P).r;
}

float normal_y(ivec2 P)
{
    return imageLoad(normal_image, P).g;
}

float H(ivec2 P)
{
    return imageLoad(displacement_image, P).r;
}

//==============================================================================================================================================================
// shader entry point
//==============================================================================================================================================================
void main()
{

    ivec2 B = ivec2(gl_NumWorkGroups.xy * gl_WorkGroupSize.xy);
    ivec2 P = ivec2(gl_GlobalInvocationID.xy);
    ivec2 Pmin = max(P - n, n - P - 1);
    ivec2 Pmax = min(P + n, 2 * B - P - n - 1);

    ivec2 xp = ivec2(Pmax.x, P.y);
    float H_xp = H(xp);
    float n_xp = normal_x(xp);

    ivec2 xm = ivec2(Pmin.x, P.y);
    float H_xm = H(xm);
    float n_xm = normal_x(xm);

    ivec2 yp = ivec2(P.x, Pmax.y);
    float H_yp = H(yp);
    float n_yp = normal_y(yp);

    ivec2 ym = ivec2(P.x, Pmin.y);
    float H_ym = H(ym);
    float n_ym = normal_y(ym);

    float h = 0.25f * (H_xp + H_xm + H_yp + H_ym) + 
              0.25f * (n_xp - n_xm + n_yp - n_ym);

    imageStore(aux_image, P, vec4(h, 0.0, 0.0, 0.0));

}