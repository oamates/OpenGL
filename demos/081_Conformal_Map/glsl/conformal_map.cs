#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

//==============================================================================================================================================================
// Every workgroup will work on 16 x 16 pixel area
// Filters that will be applied to the image will work with 5x5 surrounding window
// hence we need to load image data of size 20 x 20
//
// Predefined compute shader inputs :: 
//  const uvec3 gl_WorkGroupSize      = uvec3(local_size_x, local_size_y, local_size_z)
//  in uvec3 gl_NumWorkGroups         ----- the number of work groups passed to the dispatch function
//  in uvec3 gl_WorkGroupID           ----- the current work group for this shader invocation
//  in uvec3 gl_LocalInvocationID     ----- the current invocation of the shader within the work group
//  in uvec3 gl_GlobalInvocationID    ----- unique identifier of this invocation of the compute shader among all invocations of this compute dispatch call
//==============================================================================================================================================================

layout (rgba32f, binding = 0) uniform image2D output_image;

vec2 cmul(vec2 u, vec2 v)
{
    return vec2(u.x * v.x - u.y * v.y, u.x * v.y + u.y * v.x);
}

vec2 imul(vec2 u)
{
    return vec2(-u.y, u.x);
}

vec2 cdiv(vec2 u, vec2 v)
{
    float q = dot(v, v);
    return vec2(u.x * v.x + u.y * v.y, u.y * v.x - u.x * v.y) / q;
}


const mat2 T1 = mat2(vec2(1, 0), vec2( 1, 1));
const mat2 T2 = mat2(vec2(1, 0), vec2(-1, 1));
const mat2 S = mat2(vec2(0, 1), vec2(-1, 0));

vec2 map(vec2 z, mat2 m)
{
    //==========================================================================================================================================================
    // z = [z : 1.0];
    // to upper-half plane :: z --> w = i(z + 1) / z - 1
    //==========================================================================================================================================================
    
    vec2 w0 = imul(z + vec2(1.0, 0.0));
    vec2 w1 = z - vec2(1.0, 0.0);

    //==========================================================================================================================================================
    // SL2(Z) transform in the upper-half plane :: w --> v = (m00 * z + m10) / (m01 * z + m11)
    //==========================================================================================================================================================

    vec2 v0 = m[0][0] * w0 + m[1][0] * w1;
    vec2 v1 = m[0][1] * w0 + m[1][1] * w1;
    
    //==========================================================================================================================================================
    // back to unit circle u = (v + i) / (v - i)
    //==========================================================================================================================================================

    vec2 iv1 = imul(v1);
    vec2 u0 = v0 + iv1;
    vec2 u1 = v0 - iv1;

    return cdiv(u0, u1);
}



void main()
{
    ivec2 uv = ivec2(gl_GlobalInvocationID.xy);
    vec2 s = imageSize(output_image);
    vec2 q = vec2(uv);
    vec2 z = (2.0 * uv + 1.0) / s - 1.0;

    if (length(z) >= 1.0f)
    {
        imageStore(output_image, uv, vec4(0.0));
        return;
    }

    vec4 p = vec4(0.0f);
    mat2 m = mat2(vec2(1, 0), vec2(0, 1));

    for(int i = 0; i < 16; ++i)
    {
        mat2 m1 = m;
        for(int j = 0; j < 16; ++j)
        {
            float l = length(z);
            float q = 1.0 - l;
            p += abs(vec4(q, q * q, q * q * q, 1.0f));
            z = map(z, m1);
            m1 *= T1; 
        }

        m1 = m;
        for(int j = 0; j < 16; ++j)
        {
            float l = length(z);
            float q = 1.0 - l;
            p += abs(vec4(q, q * q, q * q * q, 1.0f));
            z = map(z, m1);
            m1 *= T2; 
        }

        m *= S;
    }

    p *= 0.625f;

    imageStore(output_image, uv, p);
    barrier();

    for(int k = 0; k < 4; ++k)
    {
        for(int i = 0; i < 16; ++i)
        {
            mat2 m1 = m;
            for(int j = 0; j < 16; ++j)
            {
                vec4 t = imageLoad(output_image, ivec2(uv * (0.5 + 0.5 * z)));
                p = 0.5 * p + t;
                z = map(z, m1);
                m1 *= T1; 
            }
        
            m1 = m;
            for(int j = 0; j < 16; ++j)
            {
                vec4 t = imageLoad(output_image, ivec2(uv * (0.5 + 0.5 * z)));
                p = 0.5 * p + t;
                z = map(z, m1);
                m1 *= T2; 
            }

            m *= S;
        }
        p *= 0.625f;

        imageStore(output_image, uv, p);
        barrier();
    }

}