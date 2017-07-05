#version 430 core

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

//==============================================================================================================================================================
// input  :: point cloud stored in GL_TEXTURE_BUFFER bound to image unit 0
// output :: unsigned integral SDF texture bound to image unit 1
//==============================================================================================================================================================
layout (r32ui, binding = 0) uniform uimage3D udf_tex;
layout (rgba32f, binding = 1) uniform imageBuffer cloud_buffer;

uniform int cloud_size;

float dot2(vec3 v) 
    { return dot(v, v); }

float triangle_udf(vec3 p, vec3 a, vec3 b, vec3 c)
{
    vec3 ba = b - a; vec3 pa = p - a;
    vec3 cb = c - b; vec3 pb = p - b;
    vec3 ac = a - c; vec3 pc = p - c;
    vec3 n = cross(ba, ac);

    float q = sign(dot(cross(ba, n), pa)) + 
              sign(dot(cross(cb, n), pb)) + 
              sign(dot(cross(ac, n), pc));

    if (q >= 2.0f) 
        return sqrt(dot(n, pa) * dot(n, pa) / dot2(n));

    return sqrt(
        min(
            min(
                dot2(ba * clamp(dot(ba, pa) / dot2(ba), 0.0f, 1.0f) - pa),
                dot2(cb * clamp(dot(cb, pb) / dot2(cb), 0.0f, 1.0f) - pb)
            ), 
            dot2(ac * clamp(dot(ac, pc) / dot2(ac), 0.0f, 1.0f) - pc)
        )
    );
}

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

const float INTEGRAL_SCALE    = 268435456.0;
const float TEXTURE_SIZE      = 256.0;
const float HALF_TEXTURE_SIZE = 0.5 * TEXTURE_SIZE;
const float TEXEL_SIZE        = 2.0 / TEXTURE_SIZE;
const float TEXEL_SIZE_DBL    = 2.0 * TEXEL_SIZE;
const float TEXEL_SHIFT       = (TEXTURE_SIZE - 1.0) / TEXTURE_SIZE;
const float TEXEL_SIZE_SQR    = 1.0 / (TEXTURE_SIZE * TEXTURE_SIZE);

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
    vec3 Pf = floor(clamp(HALF_TEXTURE_SIZE + HALF_TEXTURE_SIZE * point, 0.0, TEXTURE_SIZE));
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
            float l = norm + TEXEL_SIZE_DBL * dot(delta, vec3(shift.xyz)) + TEXEL_SIZE_SQR * float(shift.w);
            uint il = uint(INTEGRAL_SCALE * l);
            imageStore(udf_tex, idx3d, uvec4(il, 0, 0, 0));
        }
    }
}
