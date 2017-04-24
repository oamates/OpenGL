#version 400 core

uniform sampler2D value_texture;
uniform float time;
in vec2 v_texCoord2D;
in vec3 v_texCoord3D;
in vec4 v_texCoord4D;

out vec4 FragmentColor;


//===================================================================================================================================================================================================================
// Smooth interpolants between 0 (x < a) and 1 (x > b) values.
// Has zero derivative(s) at x = a and x = b up to the corresponding order (1 for hermite3, 2 for hermite5 and 3 for hermite7)
// The derivative of such polynomial must be = constant * deg_n(x) * deg_n(1 - x)
//===================================================================================================================================================================================================================

float hermite3(float a, float b, float x) { x = clamp((x - a) / (b - a), 0.0, 1.0); return x * x * (3.0 - 2.0 * x); }
vec2  hermite3(float a, float b, vec2 x)  { x = clamp((x - a) / (b - a), 0.0, 1.0); return x * x * (3.0 - 2.0 * x); }
vec3  hermite3(float a, float b, vec3 x)  { x = clamp((x - a) / (b - a), 0.0, 1.0); return x * x * (3.0 - 2.0 * x); }
vec4  hermite3(float a, float b, vec4 x)  { x = clamp((x - a) / (b - a), 0.0, 1.0); return x * x * (3.0 - 2.0 * x); }

float hermite5(float a, float b, float x) { x = clamp((x - a) / (b - a), 0.0, 1.0); return x * x * x * (10.0 + x * (6.0 * x - 15.0)); }
vec2  hermite5(float a, float b, vec2 x)  { x = clamp((x - a) / (b - a), 0.0, 1.0); return x * x * x * (10.0 + x * (6.0 * x - 15.0)); }
vec3  hermite5(float a, float b, vec3 x)  { x = clamp((x - a) / (b - a), 0.0, 1.0); return x * x * x * (10.0 + x * (6.0 * x - 15.0)); }
vec4  hermite5(float a, float b, vec4 x)  { x = clamp((x - a) / (b - a), 0.0, 1.0); return x * x * x * (10.0 + x * (6.0 * x - 15.0)); }

float hermite7(float a, float b, float x) { x = clamp((x - a) / (b - a), 0.0, 1.0); float sqr = x * x; return sqr * sqr * (35.0 - 84.0 * x + sqr * (70.0 - 20.0 * x)); }
vec2  hermite7(float a, float b, vec2 x)  { x = clamp((x - a) / (b - a), 0.0, 1.0); vec2  sqr = x * x; return sqr * sqr * (35.0 - 84.0 * x + sqr * (70.0 - 20.0 * x)); }
vec3  hermite7(float a, float b, vec3 x)  { x = clamp((x - a) / (b - a), 0.0, 1.0); vec3  sqr = x * x; return sqr * sqr * (35.0 - 84.0 * x + sqr * (70.0 - 20.0 * x)); }
vec4  hermite7(float a, float b, vec4 x)  { x = clamp((x - a) / (b - a), 0.0, 1.0); vec4  sqr = x * x; return sqr * sqr * (35.0 - 84.0 * x + sqr * (70.0 - 20.0 * x)); }

//===================================================================================================================================================================================================================
// Special case of Hermite interpolants for a = 0, b = 1.
//===================================================================================================================================================================================================================

float hermite3(float x)   { return x * x * (3.0 - 2.0 * x); }
vec2  hermite3(vec2 x)    { return x * x * (3.0 - 2.0 * x); }
vec3  hermite3(vec3 x)    { return x * x * (3.0 - 2.0 * x); }
vec4  hermite3(vec4 x)    { return x * x * (3.0 - 2.0 * x); }

float hermite3_d(float x) { return 6.0 * x * (1.0 - x); }
vec2  hermite3_d(vec2 x)  { return 6.0 * x * (1.0 - x); }
vec3  hermite3_d(vec3 x)  { return 6.0 * x * (1.0 - x); }
vec4  hermite3_d(vec4 x)  { return 6.0 * x * (1.0 - x); }

float hermite5(float x)   { return x * x * x * (10.0 + x * (6.0 * x - 15.0)); }
vec2  hermite5(vec2 x)    { return x * x * x * (10.0 + x * (6.0 * x - 15.0)); }
vec3  hermite5(vec3 x)    { return x * x * x * (10.0 + x * (6.0 * x - 15.0)); }
vec4  hermite5(vec4 x)    { return x * x * x * (10.0 + x * (6.0 * x - 15.0)); }

float hermite5_d(float x) { float q = x * (1.0 - x); return 30.0 * q * q; }
vec2  hermite5_d(vec2 x)  { vec2  q = x * (1.0 - x); return 30.0 * q * q; }
vec3  hermite5_d(vec3 x)  { vec3  q = x * (1.0 - x); return 30.0 * q * q; }
vec4  hermite5_d(vec4 x)  { vec4  q = x * (1.0 - x); return 30.0 * q * q; }

float hermite7(float x)   { float sqr = x * x; return sqr * sqr * (35.0 - 84.0 * x + sqr * (70.0 - 20.0 * x)); }
vec2  hermite7(vec2 x)    { vec2  sqr = x * x; return sqr * sqr * (35.0 - 84.0 * x + sqr * (70.0 - 20.0 * x)); }
vec3  hermite7(vec3 x)    { vec3  sqr = x * x; return sqr * sqr * (35.0 - 84.0 * x + sqr * (70.0 - 20.0 * x)); }
vec4  hermite7(vec4 x)    { vec4  sqr = x * x; return sqr * sqr * (35.0 - 84.0 * x + sqr * (70.0 - 20.0 * x)); }

float hermite7_d(float x) { float q = x * (1.0 - x); return 140.0 * q * q * q; }
vec2  hermite7_d(vec2 x)  { vec2  q = x * (1.0 - x); return 140.0 * q * q * q; }
vec3  hermite7_d(vec3 x)  { vec3  q = x * (1.0 - x); return 140.0 * q * q * q; }
vec4  hermite7_d(vec4 x)  { vec4  q = x * (1.0 - x); return 140.0 * q * q * q; }

#define TEXEL_SIZE 1.0 / 256.0
#define HALF_TEXEL 1.0 / 512.0

//===================================================================================================================================================================================================================
// 2D Value noise function : adding HALF_TEXEL is important
//===================================================================================================================================================================================================================
float vnoise(in vec2 x)
{
    vec2 p = floor(x);
    vec2 f = x - p;
    vec2 uv = p + hermite5(f);
    return texture(value_texture, TEXEL_SIZE * uv + HALF_TEXEL).x;
}

//===================================================================================================================================================================================================================
//
//  https://www.opengl.org/registry/specs/ARB/texture_gather.txt
//  ___________________________________________________________________
//  |                                                                 |
//  |   1.0  +---+---+---+---+---+---+---+---+                        |
//  |       7|   |   |   |   |   |   |   |   |                        |
//  |        +---+---+---+---+---+---+---+---+                        |
//  |       6|   |   |   |   |   | X | Y |   |                        |
//  |        +---+---+---+---+---+---+---+---+                        |
//  |       5|   |   |   |   |   | W | Z |   |                        |
//  |        +---+---+---+---+---+---+---+---+                        |
//  |       4|   |   |   |   |   |   |   |   |                        |
//  |        +---+---+---+---+---+---+---+---+                        |
//  |       3|   |   |   |   |   |   |   |   |                        |
//  |        +---+---+---+---+---+---+---+---+                        |
//  |       2|   |   |   |   |   |   |   |   |                        |
//  |        +---+---+---+---+---+---+---+---+                        |
//  |       1|   |   |   |   |   |   |   |   |                        |
//  |        +---+---+---+---+---+---+---+---+                        |
//  |       0|   |   |   |   |   |   |   |   |                        |
//  |   0.0  +---+---+---+---+---+---+---+---+                        |
//  |          0   1   2   3   4   5   6   7                          |
//  |       0.0                             1.0                       |
//  |                                                                 |
//  |  Figure 3.10a.  An example of an 8x8 texture image and the      |
//  |  components returned for textureGather.  The vector (X,Y,Z,W)   |
//  |  is returned, where each component is taken from the post-      |
//  |  swizzle R component of the corresponding texel.                |
//  |_________________________________________________________________|
//
//===================================================================================================================================================================================================================

//===================================================================================================================================================================================================================
// 2D Value noise function together with its gradient
//===================================================================================================================================================================================================================
float vnoise(in vec2 x, out vec2 dF)
{
    vec2 p = floor(x);
    vec2 f = x - p;
    vec2 blend = hermite5(f);                                                               // interpolating coefficients
    vec2 blend_d = hermite5_d(f);                                                           // derivative of the interpolation polynomials

    vec4 v = textureGather(value_texture, TEXEL_SIZE * p + HALF_TEXEL, 0);                  // get 4 surrounding lattice values : v01, v11, v10, v00

    //===============================================================================================================================================================================================================
    // value = blend_y(blend_x(v00 + v10), blend_x(v01, v11))
    // d_x(value) = blend_y(v10 - v00, v11 - v01) * d_x(blend.x)
    // d_y(value) = blend_x(v01 - v00, v11 - v10) * d_y(blend.y)
    //===============================================================================================================================================================================================================

    dF = mix(v.zx - v.ww, v.yy - v.xz, blend.yx) * blend_d;                                 // v.zx = (v10, v01), v.ww = (v00, v00), v.yy = (v11, v11), v.xz = (v01, v10) 

    vec2 blend_h = mix(v.wx, v.zy, blend.x);                                                // horizontal blend : v.wx = (v00, v01), v.zy = (v10, v11)
    return mix(blend_h.x, blend_h.y, blend.y);                                              // vertical blend
}


//===================================================================================================================================================================================================================
// 2D fractal noise based on 2D value noise function together with its gradient
//===================================================================================================================================================================================================================
float fract_vnoise(in vec2 uv, out vec2 dF)
{
    const int level = 8;
    float frequency = 1.0;
    float amplitude = 0.5;
    float v = amplitude * vnoise(uv, dF);

    for(int i = 0; i < level; ++i)
    {
        vec2 df;
        frequency *= 2.0;
        amplitude *= 0.5;    
        v += amplitude * vnoise(frequency * uv, df);
        dF += df;
    }
    dF *= 0.5;
    return v;
}

//===================================================================================================================================================================================================================
// 3D Value noise function : green channel of value_texture must be its (37, 17) - shifted red channel
//===================================================================================================================================================================================================================
float vnoise(in vec3 x)
{
    vec3 p = floor(x);
    vec3 f = x - p;
    vec3 blend = hermite5(f);
    vec2 uv = (p.xy - vec2(37.0, 17.0) * p.z) + blend.xy;
    vec2 v = texture(value_texture, TEXEL_SIZE * uv + HALF_TEXEL).rg;
    return mix(v.r, v.g, blend.z);
}

//===================================================================================================================================================================================================================
// 3D Value noise function together with its gradient
//===================================================================================================================================================================================================================
float vnoise(in vec3 x, out vec3 dF)
{
    vec3 p = floor(x);
    vec3 f = x - p;
    vec3 blend = hermite5(f);                                                               // interpolating coefficients
    vec3 blend_d = hermite5_d(f);                                                           // derivative of the interpolation polynomials 

    vec2 uv = TEXEL_SIZE * (p.xy - vec2(37.0, 17.0) * p.z) + HALF_TEXEL;

    vec4 v0 = textureGather(value_texture, uv, 0);                                          // get 4 surrounding lattice values in z = 0 plane : v010, v110, v100, v000
    vec4 v1 = textureGather(value_texture, uv, 1);                                          // get 4 surrounding lattice values in z = 1 plane : v011, v111, v101, v001

    //===============================================================================================================================================================================================================
    // value = blend_z(blend_y(blend_x(v000, v100), blend_x(v010, v110)),
    //                 blend_y(blend_x(v001, v101), blend_x(v011, v111)))
    // 
    // d_x(value) = blend_z(blend_y(v100 - v000, v110 - v010), blend_y(v101 - v001, v111 - v011)) * d_x(blend.x)
    // d_y(value) = blend_x(blend_z(v010 - v000, v011 - v001), blend_z(v110 - v100, v111 - v101)) * d_y(blend.y)
    // d_z(value) = blend_y(blend_x(v001 - v000, v101 - v100), blend_x(v011 - v010, v111 - v110)) * d_z(blend.z)
    //===============================================================================================================================================================================================================
    vec3 col0 = vec3(v0.zx, v1.w) - v0.www;
    vec3 col1 = vec3(v0.y, v1.xz) - vec3(v0.x, v1.w, v0.z);
    vec3 col2 = vec3(v1.z, v0.y, v1.x) - vec3(v1.w, v0.zx);
    vec3 col3 = v1.yyy - vec3(v1.xz, v0.y); 

    dF = mix(mix(col0, col1, blend.yzx), mix(col2, col3, blend.yzx), blend.zxy) * blend_d;

    vec4 blend_z = mix(v0, v1, blend.z);                                                    // blend in parallel z - planes
    vec2 blend_h = mix(blend_z.wx, blend_z.zy, blend.x);                                    // horizontal blend
    return mix(blend_h.x, blend_h.y, blend.y);                                              // vertical blend
}

//===================================================================================================================================================================================================================
// 3D fractal noise based on 3D value noise function together with its gradient
//===================================================================================================================================================================================================================
float fract_vnoise(in vec3 uv, out vec3 dF)
{
    const int level = 7;
    float frequency = 1.0;
    float amplitude = 0.5;
    float v = amplitude * vnoise(uv, dF);

    for(int i = 0; i < level; ++i)
    {
        vec3 df;
        frequency *= 2.0;
        amplitude *= 0.5;    
        v += amplitude * vnoise(frequency * uv, df);
        dF += df;
    }

    dF *= 0.5;
    return v;
}

//===================================================================================================================================================================================================================
// 2D gradient noise
//===================================================================================================================================================================================================================
float gnoise(in vec2 x)
{
    vec2 p = floor(x);
    vec2 f = x - p;
    vec2 blend = hermite5(f);                                                               // interpolating coefficients

    vec2 uv = TEXEL_SIZE * p + HALF_TEXEL;
    vec4 a = textureGather(value_texture, uv, 0) - 0.5;                                     // get 4 surrounding gradient x-components : a01, a11, a10, a00
    vec4 b = textureGather(value_texture, uv, 2) - 0.5;                                     // get 4 surrounding gradient y-components : b01, b11, b10, b00

    //===============================================================================================================================================================================================================
    // gradient values :: v01 = a01 *   x   + b01 * (y-1), 
    //                    v11 = a11 * (x-1) + b11 * (y-1), 
    //                    v10 = a10 * (x-1) + b10 *   y, 
    //                    v00 = a00 *   x   + b00 *   y
    // function value = blend_y(blend_x(v00, v10), blend_x(v01, v11))
    //===============================================================================================================================================================================================================

    vec4 q = f.xxyy - vec4(0.0, 1.0, 0.0, 1.0);                                             // x : x-1 : y : y-1
    vec4 v = a * q.xyyx + b * q.wwzz;                                                       // 4 dot values :: a01 * x + b01 * (y-1), a11 * (x-1) + b11 * (y-1), a10 * (x-1) + b10 * y, a00 * x + b00 * y
    vec2 blend_h = mix(v.wx, v.zy, blend.x);                                                // horizontal blend : v.wx = (v00, v01), v.zy = (v10, v11)
    const float NORMALIZER = 2.807;
    return NORMALIZER * mix(blend_h.x, blend_h.y, blend.y);                                 // vertical blend
}

//===================================================================================================================================================================================================================
// 2D fractal noise based on 2D gradient noise
//===================================================================================================================================================================================================================
float fract_gnoise(in vec2 x)
{
    const int level = 6;
    float frequency = 1.0;
    float amplitude = 0.865;
    float v = amplitude * gnoise(x);

    for(int i = 0; i < level; ++i)
    {
        frequency *= 2.0;
        amplitude *= 0.5;    
        v += amplitude * gnoise(frequency * x);
    }

    return v;
}

//===================================================================================================================================================================================================================
// 2D gradient noise together with its gradient
//===================================================================================================================================================================================================================
float gnoise(in vec2 x, out vec2 dF)
{
    vec2 p = floor(x);
    vec2 f = x - p;
    vec2 blend = hermite5(f);                                                               // interpolating coefficients
    vec2 blend_d = hermite5_d(f);                                                           // derivative of the interpolation polynomials

    vec2 uv = TEXEL_SIZE * p + HALF_TEXEL;
    vec4 a = textureGather(value_texture, uv, 0) - 0.5;                                     // get 4 surrounding gradient x-components : a01, a11, a10, a00
    vec4 b = textureGather(value_texture, uv, 2) - 0.5;                                     // get 4 surrounding gradient y-components : b01, b11, b10, b00

    //===============================================================================================================================================================================================================
    // gradient values :: v01 = a01 *   x   + b01 * (y-1)
    //                    v11 = a11 * (x-1) + b11 * (y-1)
    //                    v10 = a10 * (x-1) + b10 *   y  
    //                    v00 = a00 *   x   + b00 *   y
    //
    // function value = blend_y(blend_x(v00, v10), blend_x(v01, v11))
    //
    // d_x(value) = blend_y(blend_x(a00, a10), blend_x(a01, a11)) + blend_y(v10 - v00, v11 - v01) * blend_d.x
    // d_y(value) = blend_y(blend_x(b00, b10), blend_x(b01, b11)) + blend_x(v01 - v00, v11 - v10) * blend_d.y
    //===============================================================================================================================================================================================================

    vec4 q = f.xxyy - vec4(0.0, 1.0, 0.0, 1.0);                                             // x : x-1 : y : y-1
    vec4 v = a * q.xyyx + b * q.wwzz;                                                       // 4 dot values :: a01 * x + b01 * (y-1), a11 * (x-1) + b11 * (y-1), a10 * (x-1) + b10 * y, a00 * x + b00 * y
    vec2 blend_h = mix(v.wx, v.zy, blend.x);                                                // horizontal blend : v.wx = (v00, v01), v.zy = (v10, v11)
    vec4 diff = v.zyxy - v.wxwz;                                                            // v10 - v00 : v11 - v01 : v01 - v00 : v11 - v10
    vec4 blend_c = mix(vec4(a.wx, b.wx), vec4(a.zy, b.zy), blend.x);                        // horizontal blend of x - components
    const float NORMALIZER = 2.807;
    dF = NORMALIZER * (mix(diff.xz, diff.yw, blend.yx) * blend_d + mix(blend_c.xz, blend_c.yw, blend.y));
    return NORMALIZER * mix(blend_h.x, blend_h.y, blend.y);                                 // vertical blend
}

//===================================================================================================================================================================================================================
// 2D fractal_noise gradient based on 2D gradient noise function together with its gradient
//===================================================================================================================================================================================================================
float fract_gnoise(in vec2 x, out vec2 dF)
{
    const int level = 6;
    float frequency = 1.0;
    float amplitude = 0.5;
    float v = amplitude * gnoise(x, dF);

    for(int i = 0; i < level; ++i)
    {
        vec2 df;
        frequency *= 2.0;
        amplitude *= 0.5;    
        v += amplitude * gnoise(frequency * x, df);
        dF += df;
    }
    dF *= 0.5;
    return v;
}

//===================================================================================================================================================================================================================
// 3D gradient noise
//===================================================================================================================================================================================================================
float gnoise(in vec3 x)
{
    vec3 p = floor(x);
    vec3 f = x - p;
    vec3 blend = hermite5(f);                                                               // interpolating coefficients
    vec3 blend_d = hermite5_d(f);                                                           // derivative of the interpolation polynomials

    vec2 uv = TEXEL_SIZE * (p.xy - vec2(37.0, 17.0) * p.z) + HALF_TEXEL;
    vec4 a0 = textureGather(value_texture, uv, 0) - 0.5;                                    // get 4 surrounding gradient x-components : a010, a110, a100, a000 in z = 0 plane
    vec4 b0 = textureGather(value_texture, uv, 2) - 0.5;                                    // get 4 surrounding gradient y-components : b010, b110, b100, b000 in z = 0 plane
    vec4 a1 = textureGather(value_texture, uv, 1) - 0.5;                                    // get 4 surrounding gradient x-components : a011, a111, a101, a001 in z = 1 plane
    vec4 b1 = textureGather(value_texture, uv, 3) - 0.5;                                    // get 4 surrounding gradient y-components : b011, b111, b101, b001 in z = 1 plane
    uv += TEXEL_SIZE * vec2(49.0, -11.0);
    vec4 c0 = textureGather(value_texture, uv, 0) - 0.5;                                    // get 4 surrounding gradient z-components : c010, c110, c100, c000 in z = 0 plane
    vec4 c1 = textureGather(value_texture, uv, 1) - 0.5;                                    // get 4 surrounding gradient z-components : c011, c111, c101, c001 in z = 1 plane

    //===============================================================================================================================================================================================================
    // gradient values :: v010 = a010 *   x   + b010 * (y-1) + c010 *   z     
    //                    v110 = a110 * (x-1) + b110 * (y-1) + c110 *   z        
    //                    v100 = a100 * (x-1) + b100 *   y   + c100 *   z   
    //                    v000 = a000 *   x   + b000 *   y   + c000 *   z   
    //                    v011 = a011 *   x   + b011 * (y-1) + c011 * (z-1)   
    //                    v111 = a111 * (x-1) + b111 * (y-1) + c111 * (z-1)   
    //                    v101 = a101 * (x-1) + b101 *   y   + c101 * (z-1) 
    //                    v001 = a001 *   x   + b001 *   y   + c001 * (z-1) 
    //
    // function value = blend_z(blend_y(blend_x(v000, v100), blend_x(v010, v110)), blend_y(blend_x(v001, v101), blend_x(v011, v111)))
    //===============================================================================================================================================================================================================

    vec4 q = f.xxyy - vec4(0.0, 1.0, 0.0, 1.0);                                             // x : x-1 : y : y-1
    vec4 vz0 = a0 * q.xyyx + b0 * q.wwzz + c0 * f.z;                                        // v010 : v110 : v100 : v000 
    vec4 vz1 = a1 * q.xyyx + b1 * q.wwzz + c1 * (f.z - 1.0);                                // v011 : v111 : v101 : v001

    const float NORMALIZER = 2.807;
    vec4 v = mix(vz0, vz1, blend.z);
    vec2 blend_h = mix(v.wx, v.zy, blend.x);                                                // horizontal blend : v.wx = (v00, v01), v.zy = (v10, v11)
    return NORMALIZER * mix(blend_h.x, blend_h.y, blend.y);                                 // vertical blend
}

//===================================================================================================================================================================================================================
// 3D gradient noise together with its gradient
//===================================================================================================================================================================================================================
float gnoise(in vec3 x, out vec3 dF)
{
    vec3 p = floor(x);
    vec3 f = x - p;
    vec3 blend = hermite5(f);                                                               // interpolating coefficients
    vec3 blend_d = hermite5_d(f);                                                           // derivative of the interpolation polynomials

    vec2 uv = TEXEL_SIZE * (p.xy - vec2(37.0, 17.0) * p.z) + HALF_TEXEL;
    vec4 a0 = textureGather(value_texture, uv, 0) - 0.5;                                    // get 4 surrounding gradient x-components : a010, a110, a100, a000 in z = 0 plane
    vec4 b0 = textureGather(value_texture, uv, 2) - 0.5;                                    // get 4 surrounding gradient y-components : b010, b110, b100, b000 in z = 0 plane
    vec4 a1 = textureGather(value_texture, uv, 1) - 0.5;                                    // get 4 surrounding gradient x-components : a011, a111, a101, a001 in z = 1 plane
    vec4 b1 = textureGather(value_texture, uv, 3) - 0.5;                                    // get 4 surrounding gradient y-components : b011, b111, b101, b001 in z = 1 plane
    uv += TEXEL_SIZE * vec2(49.0, -11.0);
    vec4 c0 = textureGather(value_texture, uv, 0) - 0.5;                                    // get 4 surrounding gradient z-components : c010, c110, c100, c000 in z = 0 plane
    vec4 c1 = textureGather(value_texture, uv, 1) - 0.5;                                    // get 4 surrounding gradient z-components : c011, c111, c101, c001 in z = 1 plane

    //===============================================================================================================================================================================================================
    // gradient values :: v010 = a010 *   x   + b010 * (y-1) + c010 *   z     
    //                    v110 = a110 * (x-1) + b110 * (y-1) + c110 *   z        
    //                    v100 = a100 * (x-1) + b100 *   y   + c100 *   z   
    //                    v000 = a000 *   x   + b000 *   y   + c000 *   z   
    //                    v011 = a011 *   x   + b011 * (y-1) + c011 * (z-1)   
    //                    v111 = a111 * (x-1) + b111 * (y-1) + c111 * (z-1)   
    //                    v101 = a101 * (x-1) + b101 *   y   + c101 * (z-1) 
    //                    v001 = a001 *   x   + b001 *   y   + c001 * (z-1) 
    //
    // function value = blend_z(blend_y(blend_x(v000, v100), blend_x(v010, v110)), blend_y(blend_x(v001, v101), blend_x(v011, v111)))
    //
    // d_x(value) = blend_z(blend_y(blend_x(a000, a100), blend_x(a010, a110)), blend_y(blend_x(a001, a101), blend_x(a011, a111))) + 
    //                      blend_z(blend_y(v100 - v000, v110 - v010), blend_y(v101 - v001, v111 - v011)) * blend.x  
    // d_y(value) = blend_z(blend_y(blend_x(b000, b100), blend_x(b010, b110)), blend_y(blend_x(b001, b101), blend_x(b011, b111))) + 
    //                      blend_x(blend_z(v010 - v000, v011 - v001), blend_z(v110 - v100, v111 - v101)) * blend.y
    // d_z(value) = blend_z(blend_y(blend_x(c000, c100), blend_x(c010, c110)), blend_y(blend_x(c001, c101), blend_x(c011, c111))) +
    //                      blend_y(blend_x(v001 - v000, v101 - v100), blend_x(v011 - v010, v111 - v110)) * blend.z
    //
    //===============================================================================================================================================================================================================

    vec4 q = f.xxyy - vec4(0.0, 1.0, 0.0, 1.0);                                             // x : x-1 : y : y-1
    vec4 vz0 = a0 * q.xyyx + b0 * q.wwzz + c0 * f.z;                                        // v010 : v110 : v100 : v000 
    vec4 vz1 = a1 * q.xyyx + b1 * q.wwzz + c1 * (f.z - 1.0);                                // v011 : v111 : v101 : v001

    vec3 col0 = vec3(vz0.zx, vz1.w) - vz0.www;
    vec3 col1 = vec3(vz0.y, vz1.xz) - vec3(vz0.x, vz1.w, vz0.z);
    vec3 col2 = vec3(vz1.z, vz0.y, vz1.x) - vec3(vz1.w, vz0.zx);
    vec3 col3 = vz1.yyy - vec3(vz1.xz, vz0.y); 

    vec4 aZ = mix(a0, a1, blend.z);
    vec4 bZ = mix(b0, b1, blend.z);
    vec4 cZ = mix(c0, c1, blend.z);
    vec2 aaZ = mix(aZ.wx, aZ.zy, blend.x);
    vec2 bbZ = mix(bZ.wx, bZ.zy, blend.x);
    vec2 ccZ = mix(cZ.wx, cZ.zy, blend.x);

    const float NORMALIZER = 2.807;

    dF = NORMALIZER * (mix(vec3(aaZ.x, bbZ.x, ccZ.x), vec3(aaZ.y, bbZ.y, ccZ.y), blend.y) + 
                       mix(mix(col0, col1, blend.yzx), mix(col2, col3, blend.yzx), blend.zxy) * blend_d);

    vec4 v = mix(vz0, vz1, blend.z);
    vec2 blend_h = mix(v.wx, v.zy, blend.x);                                                // horizontal blend : v.wx = (v00, v01), v.zy = (v10, v11)
    return NORMALIZER * mix(blend_h.x, blend_h.y, blend.y);                                 // vertical blend
}

//===================================================================================================================================================================================================================
// 3D fractal_noise gradient based on 3D gradient noise function together with its gradient
//===================================================================================================================================================================================================================
float fract_gnoise(in vec3 x, out vec3 dF)
{
    const int level = 7;
    float frequency = 1.0;
    float amplitude = 0.5;
    float v = amplitude * gnoise(x, dF);

    for(int i = 0; i < level; ++i)
    {
        vec3 df;
        frequency *= 2.0;
        amplitude *= 0.5;    
        v += amplitude * gnoise(frequency * x, df);
        dF += df;
    }
    dF *= 0.5;
    return v;
}

//===================================================================================================================================================================================================================
// 2D Simplex noise function
//===================================================================================================================================================================================================================
float snoise(in vec2 p)
{
    const float F2 = 0.36602540378443864676372317075294;                                    // (sqrt(3) - 1) / 2
    const float G2 = 0.21132486540518711774542560974902;                                    // (3 - sqrt(3)) / 6
    const float H2 = 0.57735026918962576450914878050196;                                    // 1 / sqrt(3)
    const float NORMALIZER = 36.0;

    const vec2 F = vec2(F2, NORMALIZER);
    const vec4 dp = vec4(G2, -H2, G2 - 1.0, 0.0);
                                                                                            // Skew the (x, y) space to determine which cell of 2 simplices we're in
    vec2 i = floor(p + dot(p, F.xx));
    vec2 p0 = i - dot(i, dp.xx);                                                            // move the cell origin back to (x, y) space
    vec2 uv = TEXEL_SIZE * i + HALF_TEXEL;                                                  // integral part for texture lookup

    vec2 dp0 = p - p0;                                                                      // The (x, y) distances from the cell origin. For the 2D case, the simplex shape is an equilateral triangle.

    vec4 a = textureGather(value_texture, uv, 0) - 0.5;                                     // get 4 surrounding gradient x-components : a01, a11, a10, a00
    vec4 b = textureGather(value_texture, uv, 2) - 0.5;                                     // get 4 surrounding gradient y-components : b01, b11, b10, b00

    vec4 inv_norm = inversesqrt(a * a + b * b);


    vec4 dx = dp0.xxxx + dp;                                                                //    x + G2    : x - H2 : x + G2 - 1.0 : x
    vec4 dy = dp0.yyyy + dp.zyxw;                                                           // y + G2 - 1.0 : y - H2 :    y + G2    : y

    vec4 n = max(0.5 - dx * dx - dy * dy, 0.0);        
    n *= n;
    n *= n;
    
    vec4 v = n * (a * dx + b * dy);
    return NORMALIZER * dot(v, inv_norm);                                                   // sum up the contributions and scale the result to fit into [-1,1]
}

//===================================================================================================================================================================================================================
// 2D fractal noise based on 2D simplex noise function
//===================================================================================================================================================================================================================
float fract_snoise(in vec2 x)
{
    const int level = 6;
    float frequency = 1.0;
    float amplitude = 0.5;
    float v = amplitude * snoise(x);

    for(int i = 0; i < level; ++i)
    {
        frequency *= 2.0;
        amplitude *= 0.5;    
        v += amplitude * snoise(frequency * x);
    }
    return v;
}

//===================================================================================================================================================================================================================
// 2D Simplex noise function with gradient
//===================================================================================================================================================================================================================
float snoise(in vec2 p, out vec2 dF)
{
    const float F2 = 0.36602540378443864676372317075294;                                    // (sqrt(3) - 1) / 2
    const float G2 = 0.21132486540518711774542560974902;                                    // (3 - sqrt(3)) / 6
    const float H2 = 0.57735026918962576450914878050196;                                    // 1 / sqrt(3)
    const float NORMALIZER = 95.0;

    const vec2 F = vec2(F2, NORMALIZER);
    const vec4 dp = vec4(G2, -H2, G2 - 1.0, 0.0);
                                                                                            // Skew the (x, y) space to determine which cell of 2 simplices we're in
    vec2 i = floor(p + dot(p, F.xx));
    vec2 p0 = i - dot(i, dp.xx);                                                            // move the cell origin back to (x, y) space
    vec2 uv = TEXEL_SIZE * i + HALF_TEXEL;                                                  // integral part for texture lookup

    vec2 dp0 = p - p0;                                                                      // The (x, y) distances from the cell origin. For the 2D case, the simplex shape is an equilateral triangle.

    vec4 a = textureGather(value_texture, uv, 0) - 0.5;                                     // get 4 surrounding gradient x-components : a01, a11, a10, a00
    vec4 b = textureGather(value_texture, uv, 2) - 0.5;                                     // get 4 surrounding gradient y-components : b01, b11, b10, b00

    vec4 inv_norm = inversesqrt(a * a + b * b);


    vec4 dx = dp0.xxxx + dp;                                                                //    x + G2    : x - H2 : x + G2 - 1.0 : x
    vec4 dy = dp0.yyyy + dp.zyxw;                                                           // y + G2 - 1.0 : y - H2 :    y + G2    : y

    vec4 n = max(0.5 - dx * dx - dy * dy, 0.0);        
    vec4 n0 = n * n; 
    vec4 v3 = n * n0 * (a * dx + b * dy);
    n0 *= n0;

    // F = 95.0 * dot(v, inv_norm)
    // dv/dx = 4n^3 * (dn/dx) * (a * dx + b * dy) + n0 * a; dn/dx = -2.0 * dx;
    // dv/dy = 4n^3 * (dn/dy) * (a * dx + b * dy) + n0 * b; dn/dy = -2.0 * dy;

    dF = NORMALIZER * vec2(dot(n0 * a - 8.0 * dx * v3, inv_norm),
                           dot(n0 * b - 8.0 * dy * v3, inv_norm));
    
    return NORMALIZER * dot(n * v3, inv_norm);                                              // sum up the contributions and scale the result to fit into [-1,1]
}

//===================================================================================================================================================================================================================
// 2D fractal_noise gradient based on 2D gradient noise function together with its gradient
//===================================================================================================================================================================================================================
float fract_snoise(in vec2 x, out vec2 dF)
{
    const int level = 7;
    float frequency = 1.0;
    float amplitude = 0.5;
    float v = amplitude * snoise(x, dF);

    for(int i = 0; i < level; ++i)
    {
        vec2 df;
        frequency *= 2.0;
        amplitude *= 0.5;    
        v += amplitude * snoise(frequency * x, df);
        dF += df;
    }
    dF *= 0.5;
    return v;
}




void main()
{

/*
    vec3 grass = vec3(0.41, 0.97, 0.01);
    vec3 snow = vec3(0.97, 0.91, 1.04);

    vec2 dF;
    float v = fract_vnoise(0.125 * v_texCoord2D, dF);
    vec3 n = normalize(vec3(dF, -1.0));
    vec3 l = vec3(0.707, 0.0, 0.707);
    float q = 0.5 + 0.5 * dot(l, n);
    vec3 diffuse = mix(snow, grass, hermite5(v));
    FragmentColor = vec4(q * diffuse, 1.0);
*/

/*
    vec3 dF;
    float v = fract_vnoise(0.25 * v_texCoord3D, dF);
    vec3 n = normalize(dF);
    vec3 l = vec3(0.707, 0., 0.707);
    float q = 0.5 + 0.5 * dot(l, n);
    vec3 diffuse = mix(snow, grass, hermite5(v));
    FragmentColor = vec4(q * diffuse, 1.0);
*/

/*
    vec3 grass = vec3(0.41, 0.97, 0.01);
    vec3 snow = vec3(0.97, 0.91, 1.04);

    vec3 dF;
    float v = fract_vnoise(0.25f * v_texCoord3D, dF);
    dF.z = -1;
    vec3 n = -normalize(dF);
    vec3 l = -vec3(cos(time), sin(time), 0.0);
    vec3 diffuse = mix(snow, grass, hermite5(v));
    float q = 0.5 + 0.5 * dot(l, n);
    FragmentColor = vec4(q * diffuse, 1.0);
*/

/*
    float v = fract_gnoise(v_texCoord2D);
    FragmentColor = vec4(vec3(0.5 + 0.5 * v), 1.0);
*/

/*
    vec2 dF, dF_trash;
    float delta = 0.01;
    vec2 dv = vec2(delta, 0.0);
    float dvdx = gnoise(v_texCoord2D, dF);
    vec2 dF_diff = 0.5 * vec2(gnoise(v_texCoord2D + dv.xy, dF_trash) - gnoise(v_texCoord2D - dv.xy, dF_trash), 
                              gnoise(v_texCoord2D + dv.yx, dF_trash) - gnoise(v_texCoord2D - dv.yx, dF_trash)) / delta;
    float q = abs(dF.x);
    FragmentColor = vec4(q, q, q, 1.0);
*/

    /* Fractal gradient 2D noise with derivative */
//    vec3 grass = vec3(0.01, 0.41, 0.97);
//    vec3 snow = vec3(0.97, 0.91, 1.04);
//    vec2 dF;
//    float v = 0.5 + 0.5 * fract_gnoise(0.25f * v_texCoord2D, dF);
//    vec3 n = normalize(vec3(dF, -1.0));
//    vec3 l = vec3(0.707, 0., 0.707);
//    vec3 diffuse = mix(snow, grass, hermite5(v));
//    float q = 0.5 + 0.5 * dot(l, n);
//    FragmentColor = vec4(q * diffuse, 1.0);

    /* Fractal gradient 3D noise with derivative */
//    vec3 grass = vec3(0.01, 0.41, 0.97);
//    vec3 snow = vec3(0.97, 0.91, 1.04);
//    vec3 dF;
//    float v = 0.5 + 0.5 * fract_gnoise(0.125f * v_texCoord3D, dF);
//    dF.z = -1;
//    vec3 n = normalize(vec3(dF));
//    vec3 l = vec3(0.0, 1.007, 0.007);
//    vec3 diffuse = mix(snow, grass, hermite5(v));
//    float q = 0.5 + 0.5 * dot(l, n);
//    FragmentColor = vec4(q * diffuse, 1.0);

    /* Fractal simplex 2D noise */
//    vec2 dF;
//    float v = 0.5 + 0.5 * fract_snoise(0.125 * v_texCoord2D, dF);
//    FragmentColor = vec4(v,v,v, 1.0);

    /* Fractal simplex 2D noise with derivative */
    vec3 grass = vec3(0.57, 0.97, 0.06);
    vec3 snow = vec3(0.97, 0.91, 1.04);
    vec2 dF;
    float v = 0.5 + 0.5 * fract_snoise(0.125f * v_texCoord2D, dF);
    vec3 n = normalize(vec3(dF, -1.0));
    vec3 l = vec3(0.707, 0.0, 0.707);
    vec3 diffuse = mix(snow, grass, hermite5(v));
    float q = 0.5 + 0.5 * dot(l, n);
    FragmentColor = vec4(q * diffuse, 1.0);
/*
    vec2 dF, dF_trash;
    float delta = 0.01;
    vec2 dv = vec2(delta, 0.0);
    float dvdx = snoise(v_texCoord2D, dF);
    vec2 dF_diff = 0.5 * vec2(snoise(v_texCoord2D + dv.xy, dF_trash) - snoise(v_texCoord2D - dv.xy, dF_trash), 
                              snoise(v_texCoord2D + dv.yx, dF_trash) - snoise(v_texCoord2D - dv.yx, dF_trash)) / delta;
    float q = length(dF - dF_diff);
    FragmentColor = vec4(q, q, q, 1.0);
*/

}
