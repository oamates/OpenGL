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

layout (rgba32f, binding = 0) uniform image2D input_image;
layout (rgba32f, binding = 1) uniform image2D output_image;

uniform float time;

shared vec4 rgba[40][40];
shared vec4 hsvl[40][40];

//==========================================================================================================================================================
// find out who we are
//==========================================================================================================================================================
ivec2 StoreBase4x4Idx = 4 * ivec2(gl_GlobalInvocationID.xy);
ivec2 GlobalBase5x5Idx = StoreBase4x4Idx + ivec2(gl_LocalInvocationID.xy) - ivec2(4);
ivec2 LocalBase4x4Idx = 4 * ivec2(gl_LocalInvocationID.xy) + ivec2(4);
ivec2 LocalBase5x5Idx = 5 * ivec2(gl_LocalInvocationID.xy);
ivec2 BorderReflectIdx = 8 * ivec2(gl_NumWorkGroups.xy * gl_WorkGroupSize.xy) - ivec2(1);

//==============================================================================================================================================================
// uniform function pointer to choose the active filter from UI
//==============================================================================================================================================================
subroutine void image_filter4x4(void);
subroutine uniform image_filter4x4 filter4x4;

//==============================================================================================================================================================
// the output of the shader :: all filters below must fill this array in rgb space
//==============================================================================================================================================================
vec4 rgb_out[4][4];





//==============================================================================================================================================================
// auxiliary rgb --> hsv and hsv --> rgb routines
//==============================================================================================================================================================
vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}
 

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

const vec3 rgb_power = vec3(0.299f, 0.587f, 0.114f);



//==============================================================================================================================================================
// Auxiliary stuff
//==============================================================================================================================================================
int iX = LocalBase4x4Idx.x; 
int iY = LocalBase4x4Idx.y; 

float hash(vec2 p)
{
    float h = dot(p, vec2(127.1f, 311.7f)); 
    return fract(cos(h) * 43134.717f);
}

vec3 hash3(vec2 p)
{
    vec3 h = p.x * vec3(127.19f, -12.157f, 91.417f) + p.y * vec3(-513.29f, 77.441f, 13.491f);
    return fract(cos(h) * 43134.717f);
}

//==============================================================================================================================================================
// Probabilistic dithering algorithm in all components :: 256 colors :: only 8x8x4 rgb values are available
//==============================================================================================================================================================
subroutine(image_filter4x4) void dithering_rgb()                                  
{
    const vec3 Q = vec3(7.0f, 7.0f, 3.0f); 
    for (int x = 0; x < 4; ++x) for (int y = 0; y < 4; ++y)
    {
        vec3 C = rgba[iX + x][iY + y].rgb;
        vec3 sC = Q * C;
        vec3 iC = floor(sC);
        vec3 fC = sC - iC;
        vec3 h3 = hash3(StoreBase4x4Idx);
        vec3 g = iC + step(h3, fC);
        rgb_out[x][y] = vec4(g / Q, 1.0f);
    }
}

//==============================================================================================================================================================
// Probabilistic dithering algorithm on luminance :: only 17 shades of gray [0/16, 1/16, ... 16/16] are available
//==============================================================================================================================================================
subroutine(image_filter4x4) void dithering_luminance()                                
{
    const float Q = 16.0f; 
    for (int x = 0; x < 4; ++x) for (int y = 0; y < 4; ++y)
    {
        float L = hsvl[iX + x][iY + y].a;
        float sL = Q * L;
        float iL = floor(sL);
        float fL = sL - iL;
        float h = hash(StoreBase4x4Idx);
        float g = iL + step(h, fL);
        rgb_out[x][y] = vec4(vec3(g / Q), 1.0f);
    }
}

//==============================================================================================================================================================
// Constant hue filter
//==============================================================================================================================================================
subroutine(image_filter4x4) void constant_hue_filter()                                
{
    for (int x = 0; x < 4; ++x) for (int y = 0; y < 4; ++y)
    {
        vec3 q = vec3(0.7, hsvl[iX + x][iY + y].gb);
        rgb_out[x][y] = vec4(hsv2rgb(q), 1.0f);
    }
}

//==============================================================================================================================================================
// Sobel edge detector applied to luminance
//==============================================================================================================================================================
subroutine(image_filter4x4) void sobel_edge_detector()
{
    for (int x = 0; x < 4; ++x) for (int y = 0; y < 4; ++y)
    {
        vec2 g = vec2(
            (hsvl[iX + x + 1][iY + y - 1].a + 2.0f * hsvl[iX + x + 1][iY + y + 0].a + hsvl[iX + x + 1][iY + y + 1].a) - 
            (hsvl[iX + x - 1][iY + y - 1].a + 2.0f * hsvl[iX + x - 1][iY + y + 0].a + hsvl[iX + x - 1][iY + y + 1].a),
            (hsvl[iX + x - 1][iY + y + 1].a + 2.0f * hsvl[iX + x + 0][iY + y + 1].a + hsvl[iX + x + 1][iY + y + 1].a) - 
            (hsvl[iX + x - 1][iY + y - 1].a + 2.0f * hsvl[iX + x + 0][iY + y - 1].a + hsvl[iX + x + 1][iY + y - 1].a)
        );
        float l = length(g);
        rgb_out[x][y] = vec4(vec3(l), 1.0f);
    }
}

//========================================`======================================================================================================================
// Scharr edge detector applied to hue :: usually shows some bullshit
//==============================================================================================================================================================
subroutine(image_filter4x4) void scharr_edge_detector_hue()                           
{
    for (int x = 0; x < 4; ++x) for (int y = 0; y < 4; ++y)
    {
        vec2 g = vec2(
            (3.0f * hsvl[iX + x + 1][iY + y - 1].r + 10.0f * hsvl[iX + x + 1][iY + y + 0].r + 3.0f * hsvl[iX + x + 1][iY + y + 1].r) - 
            (3.0f * hsvl[iX + x - 1][iY + y - 1].r + 10.0f * hsvl[iX + x - 1][iY + y + 0].r + 3.0f * hsvl[iX + x - 1][iY + y + 1].r),
            (3.0f * hsvl[iX + x - 1][iY + y + 1].r + 10.0f * hsvl[iX + x + 0][iY + y + 1].r + 3.0f * hsvl[iX + x + 1][iY + y + 1].r) - 
            (3.0f * hsvl[iX + x - 1][iY + y - 1].r + 10.0f * hsvl[iX + x + 0][iY + y - 1].r + 3.0f * hsvl[iX + x + 1][iY + y - 1].r)
        );
        float l = 0.25f * length(g);
        rgb_out[x][y] = vec4(vec3(l), 1.0f);
    }
}

//==============================================================================================================================================================
// Scharr edge detector applied to saturation
//==============================================================================================================================================================
subroutine(image_filter4x4) void scharr_edge_detector_saturation()
{
    for (int x = 0; x < 4; ++x) for (int y = 0; y < 4; ++y)
    {
        vec2 g = vec2(
            (3.0f * hsvl[iX + x + 1][iY + y - 1].g + 10.0f * hsvl[iX + x + 1][iY + y + 0].g + 3.0f * hsvl[iX + x + 1][iY + y + 1].g) - 
            (3.0f * hsvl[iX + x - 1][iY + y - 1].g + 10.0f * hsvl[iX + x - 1][iY + y + 0].g + 3.0f * hsvl[iX + x - 1][iY + y + 1].g),
            (3.0f * hsvl[iX + x - 1][iY + y + 1].g + 10.0f * hsvl[iX + x + 0][iY + y + 1].g + 3.0f * hsvl[iX + x + 1][iY + y + 1].g) - 
            (3.0f * hsvl[iX + x - 1][iY + y - 1].g + 10.0f * hsvl[iX + x + 0][iY + y - 1].g + 3.0f * hsvl[iX + x + 1][iY + y - 1].g)
        );
        float l = 0.25f * length(g);
        rgb_out[x][y] = vec4(vec3(l), 1.0f);
    }
}    

//==============================================================================================================================================================
// Scharr edge detector applied to HSV value
//==============================================================================================================================================================
subroutine(image_filter4x4) void scharr_edge_detector_value()
{
    for (int x = 0; x < 4; ++x) for (int y = 0; y < 4; ++y)
    {
        vec2 g = vec2(
            (3.0f * hsvl[iX + x + 1][iY + y - 1].b + 10.0f * hsvl[iX + x + 1][iY + y + 0].b + 3.0f * hsvl[iX + x + 1][iY + y + 1].b) - 
            (3.0f * hsvl[iX + x - 1][iY + y - 1].b + 10.0f * hsvl[iX + x - 1][iY + y + 0].b + 3.0f * hsvl[iX + x - 1][iY + y + 1].b),
            (3.0f * hsvl[iX + x - 1][iY + y + 1].b + 10.0f * hsvl[iX + x + 0][iY + y + 1].b + 3.0f * hsvl[iX + x + 1][iY + y + 1].b) - 
            (3.0f * hsvl[iX + x - 1][iY + y - 1].b + 10.0f * hsvl[iX + x + 0][iY + y - 1].b + 3.0f * hsvl[iX + x + 1][iY + y - 1].b)
        );
        float l = 0.25f * length(g);
        rgb_out[x][y] = vec4(vec3(l), 1.0f);
    }
}    

//==============================================================================================================================================================
// Scharr edge detector applied to luminosity
//==============================================================================================================================================================
subroutine(image_filter4x4) void scharr_edge_detector_luminosity()
{
    for (int x = 0; x < 4; ++x) for (int y = 0; y < 4; ++y)
    {
        vec2 g = vec2(
            (3.0f * hsvl[iX + x + 1][iY + y - 1].a + 10.0f * hsvl[iX + x + 1][iY + y + 0].a + 3.0f * hsvl[iX + x + 1][iY + y + 1].a) - 
            (3.0f * hsvl[iX + x - 1][iY + y - 1].a + 10.0f * hsvl[iX + x - 1][iY + y + 0].a + 3.0f * hsvl[iX + x - 1][iY + y + 1].a),
            (3.0f * hsvl[iX + x - 1][iY + y + 1].a + 10.0f * hsvl[iX + x + 0][iY + y + 1].a + 3.0f * hsvl[iX + x + 1][iY + y + 1].a) - 
            (3.0f * hsvl[iX + x - 1][iY + y - 1].a + 10.0f * hsvl[iX + x + 0][iY + y - 1].a + 3.0f * hsvl[iX + x + 1][iY + y - 1].a)
        );
        float l = 0.25f * length(g);
        rgb_out[x][y] = vec4(vec3(l), 1.0f);
    }
}    

//==============================================================================================================================================================
const float gaussian5x5_filter[5][5] = 
{
    {1.0,  4.0,  7.0,  4.0, 1.0},
    {4.0, 16.0, 26.0, 16.0, 4.0},
    {7.0, 26.0, 41.0, 26.0, 7.0},
    {4.0, 16.0, 26.0, 16.0, 4.0},
    {1.0,  4.0,  7.0,  4.0, 1.0}
};

const float weight_normalizer = 1.0 / 273.0;

//==============================================================================================================================================================
// gaussian blur filter with 5x5 window in rgb space
//==============================================================================================================================================================
subroutine(image_filter4x4) void gaussian_blur5x5_rgb()
{
    for (int x = 0; x < 4; ++x) for (int y = 0; y < 4; ++y)
    {
        vec3 rgb_gb = vec3(0.0);
        for(int p = -2; p <= 2; ++p)
            for(int q = -2; q <= 2; ++q)
                rgb_gb += gaussian5x5_filter[p + 2][q + 2] * rgba[iX + x + p][iY + y + q].rgb;
        rgb_gb *= weight_normalizer;
        rgb_out[x][y] = vec4(rgb_gb, 1.0f);        
    }
}

//==============================================================================================================================================================
// gaussian blur filter with 5x5 window in hsv space
//==============================================================================================================================================================
subroutine(image_filter4x4) void gaussian_blur5x5_hsv()
{
    for (int x = 0; x < 4; ++x) for (int y = 0; y < 4; ++y)
    {
        vec3 hsv_gb = vec3(0.0);
        for(int p = -2; p <= 2; ++p)
            for(int q = -2; q <= 2; ++q)
                hsv_gb += gaussian5x5_filter[p + 2][q + 2] * hsvl[iX + x + p][iY + y + q].rgb;
        hsv_gb *= weight_normalizer;
        rgb_out[x][y] = vec4(hsv2rgb(hsv_gb), 1.0f);        
    }
}

//==============================================================================================================================================================
const float gradient5x5_filter[5][5] = 
{
    {  -3.0,   -9.0,  0.0,   9.0,  3.0},
    { -19.0,  -78.0,  0.0,  78.0, 19.0},
    { -33.0, -169.0,  0.0, 169.0, 33.0},
    { -19.0,  -78.0,  0.0,  78.0, 19.0},
    {  -3.0,   -9.0,  0.0,   9.0,  3.0} 
};

//==============================================================================================================================================================
// bump filter
//==============================================================================================================================================================
subroutine(image_filter4x4) void bump_filter()
{
    for (int x = 0; x < 4; ++x) for (int y = 0; y < 4; ++y)
    {
        vec3 hsv_dx = vec3(0.0);
        vec3 hsv_dy = vec3(0.0);

        for(int p = -2; p <= 2; ++p) for(int q = -2; q <= 2; ++q)
        {
            hsv_dx += gradient5x5_filter[p + 2][q + 2] * hsvl[iX + x + p][iY + y + q].rgb;
            hsv_dy += gradient5x5_filter[q + 2][p + 2] * hsvl[iX + x + p][iY + y + q].rgb;
        }

        vec3 q = vec3(hsv_dx.b, hsv_dy.b, 175.0);
        q = normalize(q);
        q = vec3(0.5, 0.5, 0.75) + 0.25 * q;

        rgb_out[x][y] = vec4(q, 1.0f);        
    }
}

//==============================================================================================================================================================
// luminosity bump filter
//==============================================================================================================================================================
subroutine(image_filter4x4) void bump_filter_luminosity()
{
    for (int x = 0; x < 4; ++x) for (int y = 0; y < 4; ++y)
    {
        float dx = 0.0;
        float dy = 0.0;

        for(int p = -2; p <= 2; ++p) for(int q = -2; q <= 2; ++q)
        {
            float l = hsvl[iX + x + p][iY + y + q].a;
            dx += gradient5x5_filter[p + 2][q + 2] * l;
            dy += gradient5x5_filter[q + 2][p + 2] * l;
        }

        vec3 q = vec3(dx, dy, 97.0);
        q = normalize(q);
        q = vec3(0.5) + 0.45 * q;

        rgb_out[x][y] = vec4(q, 1.0f);        
    }
}

//==============================================================================================================================================================
const float laplace5x5_filter[5][5] = 
{
    {-1.0, - 4.0,  -7.0,  -4.0, -1.0},
    {-4.0, -16.0, -26.0, -16.0, -4.0},
    {-7.0, -26.0, 296.0, -26.0, -7.0},
    {-4.0, -16.0, -26.0, -16.0, -4.0},
    {-1.0, - 4.0,  -7.0,  -4.0, -1.0}
};

//==============================================================================================================================================================
// Laplace sharpening filter
//==============================================================================================================================================================
subroutine(image_filter4x4) void laplace_sharpening()
{
    for (int x = 0; x < 4; ++x) for (int y = 0; y < 4; ++y)
    {
        float s = 0.0;

        for(int p = -2; p <= 2; ++p) for(int q = -2; q <= 2; ++q)
        {
            float l = hsvl[iX + x + p][iY + y + q].a;
            s += laplace5x5_filter[p + 2][q + 2] * l;
        }

        s /= 64.0;
        s = clamp(s, 0.0, 1.0);
        rgb_out[x][y] = vec4(vec3(s), 1.0f);        
    }
}

//==============================================================================================================================================================
// shader entry point
//==============================================================================================================================================================
void main(void)
{
    //==========================================================================================================================================================
    // load shared area of the image, simultaneuosly computing hsv components,
    // on loading non-existent boundary pixels assume that the resulting texture will be used with GL_MIRRORED_REPEAT wrap mode 
    //==========================================================================================================================================================
    int bX = LocalBase5x5Idx.x;
    int bY = LocalBase5x5Idx.y;

    for (int x = 0; x < 5; ++x)
        for (int y = 0; y < 5; ++y)
        {
            ivec2 P = GlobalBase5x5Idx + ivec2(x, y);
            P = max(P, ivec2(-1) - P);
            P = min(P, BorderReflectIdx - P);
            vec4 c = imageLoad(input_image, P);
            rgba[bX + x][bY + y] = c;
            hsvl[bX + x][bY + y] = vec4(rgb2hsv(c.rgb), dot(c.rgb, rgb_power));
        }

    //==========================================================================================================================================================
    // wait for the invocations from this workgroup to finish loading and rgb --> hsv conversion
    //==========================================================================================================================================================
    barrier(); 

    //==========================================================================================================================================================
    // the main computaion part :: this invocation is responsible for processing 4 x 4 pixel block starting at (iX, iY) and saving results to rgb_out
    //==========================================================================================================================================================
    filter4x4();    

    //==========================================================================================================================================================
    // done ... store the results into output image
    //==========================================================================================================================================================
    for (int x = 0; x < 4; ++x)
        for (int y = 0; y < 4; ++y)
            imageStore(output_image, StoreBase4x4Idx + ivec2(x, y), rgb_out[x][y]);
}
