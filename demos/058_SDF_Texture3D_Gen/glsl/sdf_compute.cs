#version 430 core

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

//==============================================================================================================================================================
// input  :: point cloud stored in GL_TEXTURE_BUFFER bound to image unit 0
// output :: unsigned integral SDF texture bound to image unit 1
//==============================================================================================================================================================
layout (r32ui, binding = 0) uniform uimage3D sdf_tex;
layout (rgba32f, binding = 1) uniform imageBuffer cloud_buffer;

uniform int cloud_size;

const ivec4 LATTICE_SHIFT[125] = ivec4[]
(
    ivec4(-2, -2, -2, 0xC), ivec4(-1, -2, -2, 0x9), ivec4( 0, -2, -2, 0x8), ivec4( 1, -2, -2, 0x9), ivec4( 2, -2, -2, 0xC), 
    ivec4(-2, -1, -2, 0x9), ivec4(-1, -1, -2, 0x6), ivec4( 0, -1, -2, 0x5), ivec4( 1, -1, -2, 0x6), ivec4( 2, -1, -2, 0x9), 
    ivec4(-2,  0, -2, 0x8), ivec4(-1,  0, -2, 0x5), ivec4( 0,  0, -2, 0x4), ivec4( 1,  0, -2, 0x5), ivec4( 2,  0, -2, 0x8), 
    ivec4(-2,  1, -2, 0x9), ivec4(-1,  1, -2, 0x6), ivec4( 0,  1, -2, 0x5), ivec4( 1,  1, -2, 0x6), ivec4( 2,  1, -2, 0x9), 
    ivec4(-2,  2, -2, 0xC), ivec4(-1,  2, -2, 0x9), ivec4( 0,  2, -2, 0x8), ivec4( 1,  2, -2, 0x9), ivec4( 2,  2, -2, 0xC), 
    ivec4(-2, -2, -1, 0x9), ivec4(-1, -2, -1, 0x6), ivec4( 0, -2, -1, 0x5), ivec4( 1, -2, -1, 0x6), ivec4( 2, -2, -1, 0x9), 
    ivec4(-2, -1, -1, 0x6), ivec4(-1, -1, -1, 0x3), ivec4( 0, -1, -1, 0x2), ivec4( 1, -1, -1, 0x3), ivec4( 2, -1, -1, 0x6), 
    ivec4(-2,  0, -1, 0x5), ivec4(-1,  0, -1, 0x2), ivec4( 0,  0, -1, 0x1), ivec4( 1,  0, -1, 0x2), ivec4( 2,  0, -1, 0x5), 
    ivec4(-2,  1, -1, 0x6), ivec4(-1,  1, -1, 0x3), ivec4( 0,  1, -1, 0x2), ivec4( 1,  1, -1, 0x3), ivec4( 2,  1, -1, 0x6), 
    ivec4(-2,  2, -1, 0x9), ivec4(-1,  2, -1, 0x6), ivec4( 0,  2, -1, 0x5), ivec4( 1,  2, -1, 0x6), ivec4( 2,  2, -1, 0x9), 
    ivec4(-2, -2,  0, 0x8), ivec4(-1, -2,  0, 0x5), ivec4( 0, -2,  0, 0x4), ivec4( 1, -2,  0, 0x5), ivec4( 2, -2,  0, 0x8), 
    ivec4(-2, -1,  0, 0x5), ivec4(-1, -1,  0, 0x2), ivec4( 0, -1,  0, 0x1), ivec4( 1, -1,  0, 0x2), ivec4( 2, -1,  0, 0x5), 
    ivec4(-2,  0,  0, 0x4), ivec4(-1,  0,  0, 0x1), ivec4( 0,  0,  0, 0x0), ivec4( 1,  0,  0, 0x1), ivec4( 2,  0,  0, 0x4), 
    ivec4(-2,  1,  0, 0x5), ivec4(-1,  1,  0, 0x2), ivec4( 0,  1,  0, 0x1), ivec4( 1,  1,  0, 0x2), ivec4( 2,  1,  0, 0x5), 
    ivec4(-2,  2,  0, 0x8), ivec4(-1,  2,  0, 0x5), ivec4( 0,  2,  0, 0x4), ivec4( 1,  2,  0, 0x5), ivec4( 2,  2,  0, 0x8), 
    ivec4(-2, -2,  1, 0x9), ivec4(-1, -2,  1, 0x6), ivec4( 0, -2,  1, 0x5), ivec4( 1, -2,  1, 0x6), ivec4( 2, -2,  1, 0x9), 
    ivec4(-2, -1,  1, 0x6), ivec4(-1, -1,  1, 0x3), ivec4( 0, -1,  1, 0x2), ivec4( 1, -1,  1, 0x3), ivec4( 2, -1,  1, 0x6), 
    ivec4(-2,  0,  1, 0x5), ivec4(-1,  0,  1, 0x2), ivec4( 0,  0,  1, 0x1), ivec4( 1,  0,  1, 0x2), ivec4( 2,  0,  1, 0x5), 
    ivec4(-2,  1,  1, 0x6), ivec4(-1,  1,  1, 0x3), ivec4( 0,  1,  1, 0x2), ivec4( 1,  1,  1, 0x3), ivec4( 2,  1,  1, 0x6), 
    ivec4(-2,  2,  1, 0x9), ivec4(-1,  2,  1, 0x6), ivec4( 0,  2,  1, 0x5), ivec4( 1,  2,  1, 0x6), ivec4( 2,  2,  1, 0x9), 
    ivec4(-2, -2,  2, 0xC), ivec4(-1, -2,  2, 0x9), ivec4( 0, -2,  2, 0x8), ivec4( 1, -2,  2, 0x9), ivec4( 2, -2,  2, 0xC), 
    ivec4(-2, -1,  2, 0x9), ivec4(-1, -1,  2, 0x6), ivec4( 0, -1,  2, 0x5), ivec4( 1, -1,  2, 0x6), ivec4( 2, -1,  2, 0x9), 
    ivec4(-2,  0,  2, 0x8), ivec4(-1,  0,  2, 0x5), ivec4( 0,  0,  2, 0x4), ivec4( 1,  0,  2, 0x5), ivec4( 2,  0,  2, 0x8), 
    ivec4(-2,  1,  2, 0x9), ivec4(-1,  1,  2, 0x6), ivec4( 0,  1,  2, 0x5), ivec4( 1,  1,  2, 0x6), ivec4( 2,  1,  2, 0x9), 
    ivec4(-2,  2,  2, 0xC), ivec4(-1,  2,  2, 0x9), ivec4( 0,  2,  2, 0x8), ivec4( 1,  2,  2, 0x9), ivec4( 2,  2,  2, 0xC)
);

const float integral_scale = 268435456.0;    // = 2^28
const float TEXEL_SIZE     = 0.0078125;      //   1.0 / 128.0
const float TEXEL_SHIFT    = 0.99609375;     // 255.0 / 256.0

void main()
{
    //==========================================================================================================================================================
    // index in the input buffer this invocation will work with
    //==========================================================================================================================================================
    int id = int(gl_GlobalInvocationID.x);
    if (id >= cloud_size) return;

    vec3 point = imageLoad(cloud_buffer, id).xyz;

    //==========================================================================================================================================================
    // determine the nearest lattice point
    //==========================================================================================================================================================
    vec3 Pf = floor(clamp(128.0 + 128.0 * point, 0.0, 255.875));
    ivec3 Pi = ivec3(Pf);
    Pf = TEXEL_SIZE * Pf - TEXEL_SHIFT;

    vec3 delta = Pf - point;
    float norm = dot(delta, delta);

    //==========================================================================================================================================================
    // update minimal distances in 5x5x5 texel window around Pi
    //==========================================================================================================================================================
    for (int idx = 0; idx < 125; ++idx)
    {
        ivec4 shift = LATTICE_SHIFT[idx];
        ivec3 idx3d = Pi + shift.xyz;

        if (all(greaterThanEqual(idx3d, ivec3(0))) && all(lessThanEqual(idx3d, ivec3(255))))
        {
            float l = norm + 2.0 * dot(delta, vec3(shift.xyz)) + TEXEL_SIZE * float(shift.w);
            uint il = uint(integral_scale * l);
            imageStore(sdf_tex, idx3d, uvec4(il, 0, 0, 0));
        }
    }
}
