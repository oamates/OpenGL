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
//
//  1920 = 2 * 2 * 2 * 2 * 2 * 2 * 3 
//  1080 = 2 * 2 * 2 * 3 * 3 * 3 * 5
//
//==============================================================================================================================================================

layout (rgba32f, binding = 0) uniform image2D scene_image;

uniform mat3 camera_matrix;
uniform vec3 camera_ws;
uniform vec2 focal_scale;
uniform sampler2D tb_tex;

const ivec2 size = ivec2(3, 3);
const int group_size = size.x * size.y;

ivec2 shift[group_size] = ivec2[group_size]
(
    ivec2( 0,  0),
    ivec2( 1,  0),
    ivec2(-1,  0),
    ivec2( 0,  1),
    ivec2( 0, -1),
    ivec2( 1,  1),
    ivec2(-1, -1),
    ivec2( 1, -1),
    ivec2(-1,  1)
);

const int MAX_STEPS = 125;
const float HORIZON = 200.0;


//==============================================================================================================================================================
// canyon distance function
//==============================================================================================================================================================
vec3 tri(in vec3 x)
    { return abs(fract(x) - 0.5); }

float map(vec3 p)
{    
    vec3 w = p;
    vec3 op = tri(p * 1.1 + tri(p.zxy * 1.1));
    float ground = p.y + 3.5 + dot(op, vec3(0.222)) * 0.3;
    p += (op - 0.25) * 0.3;
    p = cos(p * 0.315 * 1.41 + sin(p.zxy * 0.875 * 1.27));
    float canyon = (length(p) - 1.05) * 0.95;
    return min(ground, canyon);
}

float trace(in vec3 ro, in vec3 rd)
{    
    accum = 0.0;
    float t = 0.0, h;
    for(int i = 0; i < 160; i++)
    {    
        h = map(ro + rd * t);
        if(abs(h) < 0.001 * (t * 0.25 + 1.0) || t > FAR) break;
        t += h;
        if(abs(h) < 0.25) accum += (0.25 - abs(h)) / 24.0;
    }
    return min(t, FAR);
}

//==============================================================================================================================================================
// Shader entry point
//==============================================================================================================================================================

void main()
{
    //==========================================================================================================================================================
    // Create sorted list of texels that will raymarch together
    //==========================================================================================================================================================
    ivec2 Q = imageSize(scene_image);
    ivec2 P = ivec2(gl_GlobalInvocationID.xy) * size + ivec2(1, 1);
    vec2 ndc_scale = 2.0f / vec2(Q);

    vec3 view[group_size];
    float distance[group_size];
    distance[0] = 0.0;

    vec2 uv = ndc_scale * (vec2(P) + 0.5f) - 1.0f;
    vec3 v = camera_matrix * vec3(focal_scale * uv, -1.0);
    v = normalize(v);
    view[0] = v;

    for(int k = 1; k < group_size; ++k)
    {
        vec2 uv1 = uv + ndc_scale * vec2(shift[k]);
        vec3 v1 = camera_matrix * vec3(focal_scale * uv1, -1.0);
        view[k] = normalize(v1);
        float d = length(view[k] - view[0]);
        distance[k] = d;

        int l = k;
        while(d < distance[l - 1])
        {
            distance[l] = distance[l - 1];
            ivec2 s = shift[l - 1]; shift[l - 1] = shift[l]; shift[l] = s;
            vec3 w = view[l - 1]; view[l - 1] = view[l]; view[l] = s;
            l--;
        }
    }

    //==========================================================================================================================================================
    // 3x3 group raymarching
    //==========================================================================================================================================================
    vec3 ro = camera_ws;
    vec3 rd = v;

    float t = map(camera_ws);

    int k = group_size - 1;
    int s = 0;

    while((k != 0) && (s < MAX_STEPS))
    {
        h = map(camera_ws + v * t);
        while ((t * distance[k] > h) && (k >= 0))
        {
            //==================================================================================================================================================
            // finish raymarching of a single point of index k
            //==================================================================================================================================================
            float t0 = t;
            vec3 rd = view[k];
            int s0 = s;
            while(s0 < MAX_STEPS)
            {
                float h0 = map(camera_ws + rd * t);
                if(abs(h0) < 0.001 * (t0 * 0.25 + 1.0) || t0 > HORIZON) break;
                t0 += h0;
                ++s0;
            }
            --k;
        }
        ++s;
    }
        accum = 0.0;
        float t = 0.0, h;
        for(int i = 0; i < 160; i++)
        {    
        }
        return min(t, FAR);



    imageStore(scene_image, ivec2(gl_GlobalInvocationID.xy), vec4(1.0));
}