#version 430 core

uniform float time;

//========================================================================================================================================================================================================================
// The shader does horizontal FFT transform for a 2D n x n load 
//========================================================================================================================================================================================================================
layout (local_size_x = 128, local_size_y = 1, local_size_z = 1) in;

#define m 11
#define n (1 << m)

int iX = int(gl_GlobalInvocationID.x);
int iY = int(gl_GlobalInvocationID.y);

//========================================================================================================================================================================================================================
// load buffer :: the load data is a complex (packed as vec2) N x N matrix
// No bit reversing step is performed, therefore the first butterfly step should be different from the
// following and essentially perform both bit reversing and butterfly.
// It make sence to separate it in any case as for the first butterfly step roots of unity 
// are so simple that no multiplication is required at all
//========================================================================================================================================================================================================================
layout (rg32f, binding = 0) uniform imageBuffer input_buffer;
layout (rg32f, binding = 1) uniform imageBuffer inout_buffer;

shared vec2 Z[n];

//========================================================================================================================================================================================================================
// Bit reversion lookup tables
//========================================================================================================================================================================================================================
const int br4[4]     = {0x0, 0x2, 0x1, 0x3};

const int br8[8]     = {0x0, 0x4, 0x2, 0x6, 0x1, 0x5, 0x3, 0x7};

const int br16[16]   = {0x00, 0x08, 0x04, 0x0C, 0x02, 0x0A, 0x06, 0x0E, 0x01, 0x09, 0x05, 0x0D, 0x03, 0x0B, 0x07, 0x0F};

const int br32[32]   = {0x00, 0x10, 0x08, 0x18, 0x04, 0x14, 0x0C, 0x1C, 0x02, 0x12, 0x0A, 0x1A, 0x06, 0x16, 0x0E, 0x1E, 
                        0x01, 0x11, 0x09, 0x19, 0x05, 0x15, 0x0D, 0x1D, 0x03, 0x13, 0x0B, 0x1B, 0x07, 0x17, 0x0F, 0x1F};

const int br64[64]   = {0x00, 0x20, 0x10, 0x30, 0x08, 0x28, 0x18, 0x38, 0x04, 0x24, 0x14, 0x34, 0x0C, 0x2C, 0x1C, 0x3C,
                        0x02, 0x22, 0x12, 0x32, 0x0A, 0x2A, 0x1A, 0x3A, 0x06, 0x26, 0x16, 0x36, 0x0E, 0x2E, 0x1E, 0x3E, 
                        0x01, 0x21, 0x11, 0x31, 0x09, 0x29, 0x19, 0x39, 0x05, 0x25, 0x15, 0x35, 0x0D, 0x2D, 0x1D, 0x3D,
                        0x03, 0x23, 0x13, 0x33, 0x0B, 0x2B, 0x1B, 0x3B, 0x07, 0x27, 0x17, 0x37, 0x0F, 0x2F, 0x1F, 0x3F};

const int br128[128] = {0x00, 0x40, 0x20, 0x60, 0x10, 0x50, 0x30, 0x70, 0x08, 0x48, 0x28, 0x68, 0x18, 0x58, 0x38, 0x78, 
                        0x04, 0x44, 0x24, 0x64, 0x14, 0x54, 0x34, 0x74, 0x0C, 0x4C, 0x2C, 0x6C, 0x1C, 0x5C, 0x3C, 0x7C, 
                        0x02, 0x42, 0x22, 0x62, 0x12, 0x52, 0x32, 0x72, 0x0A, 0x4A, 0x2A, 0x6A, 0x1A, 0x5A, 0x3A, 0x7A, 
                        0x06, 0x46, 0x26, 0x66, 0x16, 0x56, 0x36, 0x76, 0x0E, 0x4E, 0x2E, 0x6E, 0x1E, 0x5E, 0x3E, 0x7E, 
                        0x01, 0x41, 0x21, 0x61, 0x11, 0x51, 0x31, 0x71, 0x09, 0x49, 0x29, 0x69, 0x19, 0x59, 0x39, 0x79, 
                        0x05, 0x45, 0x25, 0x65, 0x15, 0x55, 0x35, 0x75, 0x0D, 0x4D, 0x2D, 0x6D, 0x1D, 0x5D, 0x3D, 0x7D, 
                        0x03, 0x43, 0x23, 0x63, 0x13, 0x53, 0x33, 0x73, 0x0B, 0x4B, 0x2B, 0x6B, 0x1B, 0x5B, 0x3B, 0x7B, 
                        0x07, 0x47, 0x27, 0x67, 0x17, 0x57, 0x37, 0x77, 0x0F, 0x4F, 0x2F, 0x6F, 0x1F, 0x5F, 0x3F, 0x7F};

const int br256[256] = {0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0, 
                        0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8, 
                        0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4, 
                        0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC, 
                        0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2, 
                        0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA, 
                        0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6, 
                        0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE, 
                        0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1, 
                        0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9, 
                        0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5, 
                        0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD, 
                        0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3, 
                        0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB, 
                        0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7, 
                        0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF};

int bit_reverse(int q)
{
    if (gl_WorkGroupSize.x == 4) return br4[q];
    if (gl_WorkGroupSize.x == 8) return br8[q];
    if (gl_WorkGroupSize.x == 16) return br16[q];
    if (gl_WorkGroupSize.x == 32) return br32[q];
    if (gl_WorkGroupSize.x == 64) return br64[q];
    if (gl_WorkGroupSize.x == 128) return br128[q];
    if (gl_WorkGroupSize.x == 256) return br256[q];
    return 0;
}


//========================================================================================================================================================================================================================
// Complex multiplication and special cases of roots of unity of degree 2, 4, 8 and 16
//========================================================================================================================================================================================================================
const float pi = 3.141592653589793238462643383279502884197169399375105820974;
const float highest_freq = pi / (n / 2);
const float inv_sqrt2 = 0.707106781186547524400844362104849039284835937688474036588;
const float cos_pi8 = 0.923879532511286756128183189396788286822416625863642486115;
const float sin_pi8 = 0.382683432365089771728459984030398866761344562485627041433;

const vec2 root16_1 = vec2( cos_pi8, sin_pi8);
const vec2 root16_3 = vec2( sin_pi8, cos_pi8);
const vec2 root16_5 = vec2(-sin_pi8, cos_pi8);
const vec2 root16_7 = vec2(-cos_pi8, sin_pi8);

vec2 cmul(vec2 z, vec2 w)
    { return vec2(z.x * w.x - z.y * w.y, z.x * w.y + z.y * w.x); }

vec2 csqr(vec2 z)
    { return vec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y); }

vec2 imul(vec2 z)
    { return vec2(-z.y, z.x); }

vec2 cmr8(vec2 z)
    { return inv_sqrt2 * vec2(z.x - z.y, z.x + z.y); }

vec2 cm83(vec2 z)
    { return inv_sqrt2 * vec2(-z.x - z.y, z.x - z.y); }

vec2 iexp(float theta)
    { return sin(vec2(theta + 0.5 * pi, theta)); }

vec2 load(int index)
    { return imageLoad(input_buffer, index).rg; }

void store(int index, vec2 value)
    { imageStore(inout_buffer, index, vec4(value, 0.0, 0.0)); }

//========================================================================================================================================================================================================================
// Butterfly operations
//========================================================================================================================================================================================================================
void butterfly_2(in vec2 u0, in vec2 u1, out vec2 v0, out vec2 v1)
{
    v0 = u0 + u1;
    v1 = u0 - u1;    
}

void butterfly_4( in vec2 u0,  in vec2 u1,  in vec2 u2,  in vec2 u3,  
                 out vec2 v0, out vec2 v1, out vec2 v2, out vec2 v3)
{
    butterfly_2(u0, u1, v0, v1);
    butterfly_2(u2, u3, v2, v3);
}

void butterfly_8( in vec2 u0,  in vec2 u1,  in vec2 u2,  in vec2 u3,  in vec2 u4,  in vec2 u5,  in vec2 u6,  in vec2 u7,  
                 out vec2 v0, out vec2 v1, out vec2 v2, out vec2 v3, out vec2 v4, out vec2 v5, out vec2 v6, out vec2 v7)
{
    butterfly_4(u0, u1, u2, u3, v0, v1, v2, v3);
    butterfly_4(u4, u5, u6, u7, v4, v5, v6, v7);
}

void butterfly_16( in vec2 u0,  in vec2 u1,  in vec2 u2,  in vec2 u3,  in vec2 u4,  in vec2 u5,  in vec2 u6,  in vec2 u7,  
                   in vec2 u8,  in vec2 u9,  in vec2 uA,  in vec2 uB,  in vec2 uC,  in vec2 uD,  in vec2 uE,  in vec2 uF,
                  out vec2 v0, out vec2 v1, out vec2 v2, out vec2 v3, out vec2 v4, out vec2 v5, out vec2 v6, out vec2 v7,
                  out vec2 v8, out vec2 v9, out vec2 vA, out vec2 vB, out vec2 vC, out vec2 vD, out vec2 vE, out vec2 vF)
{
    butterfly_8(u0, u1, u2, u3, u4, u5, u6, u7, v0, v1, v2, v3, v4, v5, v6, v7);
    butterfly_8(u8, u9, uA, uB, uC, uD, uE, uF, v8, v9, vA, vB, vC, vD, vE, vF);
}





//========================================================================================================================================================================================================================
// Initial routines processing 16 elements at once : 1xFFT16, 2xFFT8, 4xFFT4 or 8xFFT2
// Should be used for n = 16 * local_size_x
//========================================================================================================================================================================================================================

void iFFT16_x1()
{
    vec2 u0, u1, u2, u3, u4, u5, u6, u7, u8, u9, uA, uB, uC, uD, uE, uF, 
         v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, vA, vB, vC, vD, vE, vF;

    int b = (iY << m) + bit_reverse(iX);

    u0 = load(b + (br16[0x0] << (m - 4))); u1 = load(b + (br16[0x1] << (m - 4)));
    u2 = load(b + (br16[0x2] << (m - 4))); u3 = load(b + (br16[0x3] << (m - 4)));
    u4 = load(b + (br16[0x4] << (m - 4))); u5 = load(b + (br16[0x5] << (m - 4)));
    u6 = load(b + (br16[0x6] << (m - 4))); u7 = load(b + (br16[0x7] << (m - 4)));
    u8 = load(b + (br16[0x8] << (m - 4))); u9 = load(b + (br16[0x9] << (m - 4)));
    uA = load(b + (br16[0xA] << (m - 4))); uB = load(b + (br16[0xB] << (m - 4)));
    uC = load(b + (br16[0xC] << (m - 4))); uD = load(b + (br16[0xD] << (m - 4)));
    uE = load(b + (br16[0xE] << (m - 4))); uF = load(b + (br16[0xF] << (m - 4)));

    butterfly_16(u0, u1, u2, u3, u4, u5, u6, u7, u8, u9, uA, uB, uC, uD, uE, uF, 
                 v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, vA, vB, vC, vD, vE, vF);

    v3 = imul(v3);
    v7 = imul(v7);
    vB = imul(vB);
    vF = imul(vF);

    butterfly_16(v0, v2, v1, v3, v4, v6, v5, v7, v8, vA, v9, vB, vC, vE, vD, vF,
                 u0, u2, u1, u3, u4, u6, u5, u7, u8, uA, u9, uB, uC, uE, uD, uF);

    u5 = cmr8(u5); 
    u6 = imul(u6); 
    u7 = cm83(u7);
    uD = cmr8(uD); 
    uE = imul(uE); 
    uF = cm83(uF);

    butterfly_16(u0, u4, u1, u5, u2, u6, u3, u7, u8, uC, u9, uD, uA, uE, uB, uF,
                 v0, v4, v1, v5, v2, v6, v3, v7, v8, vC, v9, vD, vA, vE, vB, vF);

    v9 = cmul(v9, root16_1);
    vA = cmr8(vA);
    vB = cmul(vB, root16_3);
    vC = imul(vC);
    vD = cmul(vD, root16_5);
    vE = cm83(vE);
    vF = cmul(vF, root16_7);

    butterfly_16(v0, v8, v1, v9, v2, vA, v3, vB, v4, vC, v5, vD, v6, vE, v7, vF,
                 u0, u8, u1, u9, u2, uA, u3, uB, u4, uC, u5, uD, u6, uE, u7, uF);

    b = iX << 4;
    Z[b + 0x0] = u0; Z[b + 0x1] = u1; Z[b + 0x2] = u2; Z[b + 0x3] = u3;
    Z[b + 0x4] = u4; Z[b + 0x5] = u5; Z[b + 0x6] = u6; Z[b + 0x7] = u7;
    Z[b + 0x8] = u8; Z[b + 0x9] = u9; Z[b + 0xA] = uA; Z[b + 0xB] = uB;
    Z[b + 0xC] = uC; Z[b + 0xD] = uD; Z[b + 0xE] = uE; Z[b + 0xF] = uF;
}

void iFFT8_x1()
{
    vec2 u0, u1, u2, u3, u4, u5, u6, u7, 
         v0, v1, v2, v3, v4, v5, v6, v7;

    int b = (iY << m) + bit_reverse(iX);

    u0 = load(b + (br8[0] << (m - 3))); u1 = load(b + (br8[1] << (m - 3)));
    u2 = load(b + (br8[2] << (m - 3))); u3 = load(b + (br8[3] << (m - 3)));
    u4 = load(b + (br8[4] << (m - 3))); u5 = load(b + (br8[5] << (m - 3)));
    u6 = load(b + (br8[6] << (m - 3))); u7 = load(b + (br8[7] << (m - 3)));

    butterfly_8(u0, u1, u2, u3, u4, u5, u6, u7, 
                v0, v1, v2, v3, v4, v5, v6, v7);    

    v3 = imul(v3);
    v7 = imul(v7);

    butterfly_8(v0, v2, v1, v3, v4, v6, v5, v7,
                u0, u2, u1, u3, u4, u6, u5, u7);

    u5 = cmr8(u5); 
    u6 = imul(u6); 
    u7 = cm83(u7);

    butterfly_8(u0, u4, u1, u5, u2, u6, u3, u7,
                v0, v4, v1, v5, v2, v6, v3, v7);    

    b = iX << 3;
    Z[b + 0] = v0; Z[b + 1] = v1; Z[b + 2] = v2; Z[b + 3] = v3; 
    Z[b + 4] = v4; Z[b + 5] = v5; Z[b + 6] = v6; Z[b + 7] = v7;
}

void iFFT8_x2()
{
    vec2 u0, u1, u2, u3, u4, u5, u6, u7, u8, u9, uA, uB, uC, uD, uE, uF, 
         v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, vA, vB, vC, vD, vE, vF;

    int b = (iY << m) + bit_reverse(iX);

    u0 = load(b + (br16[0x0] << (m - 4))); u1 = load(b + (br16[0x1] << (m - 4)));
    u2 = load(b + (br16[0x2] << (m - 4))); u3 = load(b + (br16[0x3] << (m - 4)));
    u4 = load(b + (br16[0x4] << (m - 4))); u5 = load(b + (br16[0x5] << (m - 4)));
    u6 = load(b + (br16[0x6] << (m - 4))); u7 = load(b + (br16[0x7] << (m - 4)));
    u8 = load(b + (br16[0x8] << (m - 4))); u9 = load(b + (br16[0x9] << (m - 4)));
    uA = load(b + (br16[0xA] << (m - 4))); uB = load(b + (br16[0xB] << (m - 4)));
    uC = load(b + (br16[0xC] << (m - 4))); uD = load(b + (br16[0xD] << (m - 4)));
    uE = load(b + (br16[0xE] << (m - 4))); uF = load(b + (br16[0xF] << (m - 4)));

    butterfly_16(u0, u1, u2, u3, u4, u5, u6, u7, u8, u9, uA, uB, uC, uD, uE, uF, 
                 v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, vA, vB, vC, vD, vE, vF);

    v3 = imul(v3);
    v7 = imul(v7);
    vB = imul(vB);
    vF = imul(vF);

    butterfly_16(v0, v2, v1, v3, v4, v6, v5, v7, v8, vA, v9, vB, vC, vE, vD, vF,
                 u0, u2, u1, u3, u4, u6, u5, u7, u8, uA, u9, uB, uC, uE, uD, uF);

    u5 = cmr8(u5); 
    u6 = imul(u6); 
    u7 = cm83(u7);
    uD = cmr8(uD); 
    uE = imul(uE); 
    uF = cm83(uF);

    butterfly_16(u0, u4, u1, u5, u2, u6, u3, u7, u8, uC, u9, uD, uA, uE, uB, uF,
                 v0, v4, v1, v5, v2, v6, v3, v7, v8, vC, v9, vD, vA, vE, vB, vF);

    b = iX << 4;
    Z[b + 0x0] = v0; Z[b + 0x1] = v1; Z[b + 0x2] = v2; Z[b + 0x3] = v3;
    Z[b + 0x4] = v4; Z[b + 0x5] = v5; Z[b + 0x6] = v6; Z[b + 0x7] = v7;
    Z[b + 0x8] = v8; Z[b + 0x9] = v9; Z[b + 0xA] = vA; Z[b + 0xB] = vB;
    Z[b + 0xC] = vC; Z[b + 0xD] = vD; Z[b + 0xE] = vE; Z[b + 0xF] = vF;
}


void iFFT4_x1()
{
    vec2 u0, u1, u2, u3, v0, v1, v2, v3;

    int b = (iY << m) + bit_reverse(iX);

    u0 = load(b + (br4[0] << (m - 2))); u1 = load(b + (br4[1] << (m - 2)));
    u2 = load(b + (br4[2] << (m - 2))); u3 = load(b + (br4[3] << (m - 2)));

    butterfly_4(u0, u1, u2, u3, 
                v0, v1, v2, v3);    

    v3 = imul(v3);

    butterfly_4(v0, v2, v1, v3,
                u0, u2, u1, u3);

    b = iX << 2;
    Z[b + 0] = u0; Z[b + 1] = u1; Z[b + 2] = u2; Z[b + 3] = u3;
}

void iFFT2_x1()
{
    int b = (iY << m) + bit_reverse(iX);
    
    vec2 u0 = load(b);
    vec2 u1 = load(b + 1 << (m - 2));

    b = iX << 1;
    Z[b] = u0 + u1;
    Z[b + 1] = u0 - u1;
}

//========================================================================================================================================================================================================================
//========================================================================================================================================================================================================================
//========================================================================================================================================================================================================================

void mFFT16_x1(int p2, float freq)
{
    int mask = p2 - 1;
    int h = iX & ~mask;
    int b = 15 * h + iX;
    float theta = freq * (iX & mask);

    vec2    w = iexp(theta); 
    vec2   rw = cmul(w, root16_1); 
    vec2   sw = cmr8(w);
    vec2  srw = cmul(w, root16_3); 
    vec2   iw = imul(w);
    vec2  irw = cmul(w, root16_5); 
    vec2  isw = cm83(w);
    vec2 isrw = cmul(w, root16_7);

    vec2   w2 = csqr(w);
    vec2  sw2 = cmr8(w2);
    vec2  iw2 = imul(w2);
    vec2 isw2 = cm83(w2);

    vec2   w4 = csqr(w2);
    vec2  iw4 = imul(w4);

    vec2   w8 = csqr(w4);

    barrier();     

    vec2 u0, u1, u2, u3, u4, u5, u6, u7, u8, u9, uA, uB, uC, uD, uE, uF, 
         v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, vA, vB, vC, vD, vE, vF;

    u0 = Z[b + 0x0 * p2]; u1 = Z[b + 0x1 * p2]; u2 = Z[b + 0x2 * p2]; u3 = Z[b + 0x3 * p2]; 
    u4 = Z[b + 0x4 * p2]; u5 = Z[b + 0x5 * p2]; u6 = Z[b + 0x6 * p2]; u7 = Z[b + 0x7 * p2]; 
    u8 = Z[b + 0x8 * p2]; u9 = Z[b + 0x9 * p2]; uA = Z[b + 0xA * p2]; uB = Z[b + 0xB * p2]; 
    uC = Z[b + 0xC * p2]; uD = Z[b + 0xD * p2]; uE = Z[b + 0xE * p2]; uF = Z[b + 0xF * p2]; 

    u1 = cmul(w8, u1);
    u3 = cmul(w8, u3);
    u5 = cmul(w8, u5);
    u7 = cmul(w8, u7); 
    u9 = cmul(w8, u9); 
    uB = cmul(w8, uB); 
    uD = cmul(w8, uD); 
    uF = cmul(w8, uF); 

    butterfly_16(u0, u1, u2, u3, u4, u5, u6, u7, u8, u9, uA, uB, uC, uD, uE, uF, 
                 v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, vA, vB, vC, vD, vE, vF);    

    v2 = cmul( w4, v2);
    v3 = cmul(iw4, v3);
    v6 = cmul( w4, v6);
    v7 = cmul(iw4, v7);
    vA = cmul( w4, vA);
    vB = cmul(iw4, vB);
    vE = cmul( w4, vE);
    vF = cmul(iw4, vF);

    butterfly_16(v0, v2, v1, v3, v4, v6, v5, v7, v8, vA, v9, vB, vC, vE, vD, vF,
                 u0, u2, u1, u3, u4, u6, u5, u7, u8, uA, u9, uB, uC, uE, uD, uF);

    u4 = cmul(  w2, u4); 
    u5 = cmul( sw2, u5); 
    u6 = cmul( iw2, u6); 
    u7 = cmul(isw2, u7);
    uC = cmul(  w2, uC); 
    uD = cmul( sw2, uD); 
    uE = cmul( iw2, uE); 
    uF = cmul(isw2, uF);

    butterfly_16(u0, u4, u1, u5, u2, u6, u3, u7, u8, uC, u9, uD, uA, uE, uB, uF,
                 v0, v4, v1, v5, v2, v6, v3, v7, v8, vC, v9, vD, vA, vE, vB, vF);

    v8 = cmul(   w, v8); 
    v9 = cmul(  rw, v9); 
    vA = cmul(  sw, vA);
    vB = cmul( srw, vB); 
    vC = cmul(  iw, vC);
    vD = cmul( irw, vD); 
    vE = cmul( isw, vE);
    vF = cmul(isrw, vF);

    butterfly_16(v0, v8, v1, v9, v2, vA, v3, vB, v4, vC, v5, vD, v6, vE, v7, vF,
                 u0, u8, u1, u9, u2, uA, u3, uB, u4, uC, u5, uD, u6, uE, u7, uF);     

    Z[b + 0x0 * p2] = u0; Z[b + 0x1 * p2] = u1; Z[b + 0x2 * p2] = u2; Z[b + 0x3 * p2] = u3;
    Z[b + 0x4 * p2] = u4; Z[b + 0x5 * p2] = u5; Z[b + 0x6 * p2] = u6; Z[b + 0x7 * p2] = u7;
    Z[b + 0x8 * p2] = u8; Z[b + 0x9 * p2] = u9; Z[b + 0xA * p2] = uA; Z[b + 0xB * p2] = uB;
    Z[b + 0xC * p2] = uC; Z[b + 0xD * p2] = uD; Z[b + 0xE * p2] = uE; Z[b + 0xF * p2] = uF;       
}

void mFFT8_x1(int p2, float freq)
{
    int mask = p2 - 1;
    int h = iX & ~mask;
    int b = 7 * h + iX;
    float theta = freq * (iX & mask);

    vec2   w = iexp(theta); 
    vec2  sw = cmr8(w);
    vec2  iw = imul(w);
    vec2 isw = cm83(w);

    vec2  w2 = csqr(w);
    vec2 iw2 = imul(w2);

    vec2  w4 = csqr(w2);

    barrier();

    vec2 u0, u1, u2, u3, u4, u5, u6, u7, 
         v0, v1, v2, v3, v4, v5, v6, v7;

    u0 = Z[b + 0 * p2]; u1 = Z[b + 1 * p2]; u2 = Z[b + 2 * p2]; u3 = Z[b + 3 * p2];
    u4 = Z[b + 4 * p2]; u5 = Z[b + 5 * p2]; u6 = Z[b + 6 * p2]; u7 = Z[b + 7 * p2];

    u1 = cmul(w4, u1);
    u3 = cmul(w4, u3);
    u5 = cmul(w4, u5);
    u7 = cmul(w4, u7); 

    butterfly_8(u0, u1, u2, u3, u4, u5, u6, u7,
                v0, v1, v2, v3, v4, v5, v6, v7);

    v2 = cmul( w2, v2);
    v3 = cmul(iw2, v3);
    v6 = cmul( w2, v6);
    v7 = cmul(iw2, v7);

    butterfly_8(v0, v2, v1, v3, v4, v6, v5, v7,
                u0, u2, u1, u3, u4, u6, u5, u7);

    u4 = cmul(  w, u4);
    u5 = cmul( sw, u5);
    u6 = cmul( iw, u6);
    u7 = cmul(isw, u7);

    butterfly_8(u0, u4, u1, u5, u2, u6, u3, u7,
                v0, v4, v1, v5, v2, v6, v3, v7); 

    Z[b + 0 * p2] = v0; Z[b + 1 * p2] = v1; Z[b + 2 * p2] = v2; Z[b + 3 * p2] = v3;
    Z[b + 4 * p2] = v4; Z[b + 5 * p2] = v5; Z[b + 6 * p2] = v6; Z[b + 7 * p2] = v7;        
}

void mFFT4_x1(int p2, float freq)
{
    int mask = p2 - 1;
    int h = iX & ~mask;
    int b = 3 * h + iX;
    float theta = freq * (iX & mask);

    vec2  w = iexp(theta); 
    vec2 iw = imul(w);
    vec2 w2 = csqr(w);
 
    barrier();

    vec2 u0, u1, u2, u3, 
         v0, v1, v2, v3;         

    u0 = Z[b + 0 * p2]; u1 = Z[b + 1 * p2]; u2 = Z[b + 2 * p2]; u3 = Z[b + 3 * p2]; 

    u1 = cmul(w2, u1);
    u3 = cmul(w2, u3);

    butterfly_4(u0, u1, u2, u3, 
                v0, v1, v2, v3);

    v2 = cmul( w, v2);
    v3 = cmul(iw, v3);

    butterfly_4(v0, v2, v1, v3,
                u0, u2, u1, u3);

    Z[b + 0 * p2] = u0; Z[b + 1 * p2] = u1; Z[b + 2 * p2] = u2; Z[b + 3 * p2] = u3;      
}

void mFFT2_x1(int p2, float freq)
{
    int mask = p2 - 1;
    int h = iX & ~mask;
    int b = h + iX;
    float theta = freq * (iX & mask);

    vec2 w = iexp(theta); 
    barrier();

    vec2 u0 = Z[b]; 
    vec2 u1 = Z[b + p2];

    u1 = cmul(w, u1);

    Z[b] = u0 + u1;
    Z[b + p2] = u0 - u1;     
}

//========================================================================================================================================================================================================================
//========================================================================================================================================================================================================================

void fFFT16_x1()
{
    float theta = highest_freq * iX;

    vec2    w = iexp(theta); 
    vec2   rw = cmul(w, root16_1); 
    vec2   sw = cmr8(w);
    vec2  srw = cmul(w, root16_3); 
    vec2   iw = imul(w);
    vec2  irw = cmul(w, root16_5); 
    vec2  isw = cm83(w);
    vec2 isrw = cmul(w, root16_7);

    vec2   w2 = csqr(w);
    vec2  sw2 = cmr8(w2);
    vec2  iw2 = imul(w2);
    vec2 isw2 = cm83(w2);

    vec2   w4 = csqr(w2);
    vec2  iw4 = imul(w4);

    vec2   w8 = csqr(w4);

    barrier();     

    vec2 u0, u1, u2, u3, u4, u5, u6, u7, u8, u9, uA, uB, uC, uD, uE, uF, 
         v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, vA, vB, vC, vD, vE, vF;

    u0 = Z[iX + (0x0 << (m - 4))]; u1 = Z[iX + (0x1 << (m - 4))]; u2 = Z[iX + (0x2 << (m - 4))]; u3 = Z[iX + (0x3 << (m - 4))]; 
    u4 = Z[iX + (0x4 << (m - 4))]; u5 = Z[iX + (0x5 << (m - 4))]; u6 = Z[iX + (0x6 << (m - 4))]; u7 = Z[iX + (0x7 << (m - 4))]; 
    u8 = Z[iX + (0x8 << (m - 4))]; u9 = Z[iX + (0x9 << (m - 4))]; uA = Z[iX + (0xA << (m - 4))]; uB = Z[iX + (0xB << (m - 4))]; 
    uC = Z[iX + (0xC << (m - 4))]; uD = Z[iX + (0xD << (m - 4))]; uE = Z[iX + (0xE << (m - 4))]; uF = Z[iX + (0xF << (m - 4))]; 

    u1 = cmul(w8, u1);
    u3 = cmul(w8, u3);
    u5 = cmul(w8, u5);
    u7 = cmul(w8, u7); 
    u9 = cmul(w8, u9); 
    uB = cmul(w8, uB); 
    uD = cmul(w8, uD); 
    uF = cmul(w8, uF); 

    butterfly_16(u0, u1, u2, u3, u4, u5, u6, u7, u8, u9, uA, uB, uC, uD, uE, uF, 
                 v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, vA, vB, vC, vD, vE, vF);    

    v2 = cmul( w4, v2);
    v3 = cmul(iw4, v3);
    v6 = cmul( w4, v6);
    v7 = cmul(iw4, v7);
    vA = cmul( w4, vA);
    vB = cmul(iw4, vB);
    vE = cmul( w4, vE);
    vF = cmul(iw4, vF);

    butterfly_16(v0, v2, v1, v3, v4, v6, v5, v7, v8, vA, v9, vB, vC, vE, vD, vF,
                 u0, u2, u1, u3, u4, u6, u5, u7, u8, uA, u9, uB, uC, uE, uD, uF);

    u4 = cmul(  w2, u4); 
    u5 = cmul( sw2, u5); 
    u6 = cmul( iw2, u6); 
    u7 = cmul(isw2, u7);
                                                                                      
    uC = cmul(  w2, uC); 
    uD = cmul( sw2, uD); 
    uE = cmul( iw2, uE); 
    uF = cmul(isw2, uF);

    butterfly_16(u0, u4, u1, u5, u2, u6, u3, u7, u8, uC, u9, uD, uA, uE, uB, uF,
                 v0, v4, v1, v5, v2, v6, v3, v7, v8, vC, v9, vD, vA, vE, vB, vF);

    v8 = cmul(   w, v8); 
    v9 = cmul(  rw, v9); 
    vA = cmul(  sw, vA);
    vB = cmul( srw, vB); 
    vC = cmul(  iw, vC);
    vD = cmul( irw, vD); 
    vE = cmul( isw, vE);
    vF = cmul(isrw, vF);

    butterfly_16(v0, v8, v1, v9, v2, vA, v3, vB, v4, vC, v5, vD, v6, vE, v7, vF,
                 u0, u8, u1, u9, u2, uA, u3, uB, u4, uC, u5, uD, u6, uE, u7, uF);

    int b = (iY << m) + iX;

    store(b + (0x0 << (m - 4)), u0); store(b + (0x1 << (m - 4)), u1);
    store(b + (0x2 << (m - 4)), u2); store(b + (0x3 << (m - 4)), u3);
    store(b + (0x4 << (m - 4)), u4); store(b + (0x5 << (m - 4)), u5);
    store(b + (0x6 << (m - 4)), u6); store(b + (0x7 << (m - 4)), u7);
    store(b + (0x8 << (m - 4)), u8); store(b + (0x9 << (m - 4)), u9);
    store(b + (0xA << (m - 4)), uA); store(b + (0xB << (m - 4)), uB);
    store(b + (0xC << (m - 4)), uC); store(b + (0xD << (m - 4)), uD);
    store(b + (0xE << (m - 4)), uE); store(b + (0xF << (m - 4)), uF);                
}


void fFFT8_x2()
{
    float theta = highest_freq * iX;

    vec2    w = iexp(theta); 
    vec2   rw = cmul(w, root16_1); 
    vec2   sw = cmr8(w);
    vec2  srw = cmul(w, root16_3); 
    vec2   iw = imul(w);
    vec2  irw = cmul(w, root16_5); 
    vec2  isw = cm83(w);
    vec2 isrw = cmul(w, root16_7);

    vec2   w2 = csqr(w);
    vec2  sw2 = cmr8(w2);
    vec2  iw2 = imul(w2);
    vec2 isw2 = cm83(w2);

    vec2   w4 = csqr(w2);
    vec2  iw4 = imul(w4);

    barrier();     

    vec2 u0, u1, u2, u3, u4, u5, u6, u7, u8, u9, uA, uB, uC, uD, uE, uF, 
         v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, vA, vB, vC, vD, vE, vF;

    u0 = Z[iX + (0x0 << (m - 4))]; u1 = Z[iX + (0x1 << (m - 4))]; u2 = Z[iX + (0x2 << (m - 4))]; u3 = Z[iX + (0x3 << (m - 4))]; 
    u4 = Z[iX + (0x4 << (m - 4))]; u5 = Z[iX + (0x5 << (m - 4))]; u6 = Z[iX + (0x6 << (m - 4))]; u7 = Z[iX + (0x7 << (m - 4))]; 
    u8 = Z[iX + (0x8 << (m - 4))]; u9 = Z[iX + (0x9 << (m - 4))]; uA = Z[iX + (0xA << (m - 4))]; uB = Z[iX + (0xB << (m - 4))]; 
    uC = Z[iX + (0xC << (m - 4))]; uD = Z[iX + (0xD << (m - 4))]; uE = Z[iX + (0xE << (m - 4))]; uF = Z[iX + (0xF << (m - 4))]; 

    u2 = cmul( w4, u2);
    u3 = cmul(iw4, u3);
    u6 = cmul( w4, u6);
    u7 = cmul(iw4, u7);
    uA = cmul( w4, uA);
    uB = cmul(iw4, uB);
    uE = cmul( w4, uE);
    uF = cmul(iw4, uF);

    butterfly_16(u0, u2, u1, u3, u4, u6, u5, u7, u8, uA, u9, uB, uC, uE, uD, uF,
                 v0, v2, v1, v3, v4, v6, v5, v7, v8, vA, v9, vB, vC, vE, vD, vF);

    v4 = cmul(  w2, v4); 
    v5 = cmul( sw2, v5); 
    v6 = cmul( iw2, v6); 
    v7 = cmul(isw2, v7);
    vC = cmul(  w2, vC); 
    vD = cmul( sw2, vD); 
    vE = cmul( iw2, vE); 
    vF = cmul(isw2, vF);

    butterfly_16(v0, v4, v1, v5, v2, v6, v3, v7, v8, vC, v9, vD, vA, vE, vB, vF, 
                 u0, u4, u1, u5, u2, u6, u3, u7, u8, uC, u9, uD, uA, uE, uB, uF);

    u8 = cmul(   w, u8); 
    u9 = cmul(  rw, u9); 
    uA = cmul(  sw, uA);
    uB = cmul( srw, uB); 
    uC = cmul(  iw, uC);
    uD = cmul( irw, uD); 
    uE = cmul( isw, uE);
    uF = cmul(isrw, uF);

    butterfly_16(u0, u8, u1, u9, u2, uA, u3, uB, u4, uC, u5, uD, u6, uE, u7, uF,
                 v0, v8, v1, v9, v2, vA, v3, vB, v4, vC, v5, vD, v6, vE, v7, vF);

    int b = (iY << m) + iX;

    store(b + (0x0 << (m - 4)), v0); store(b + (0x1 << (m - 4)), v1);
    store(b + (0x2 << (m - 4)), v2); store(b + (0x3 << (m - 4)), v3);
    store(b + (0x4 << (m - 4)), v4); store(b + (0x5 << (m - 4)), v5);
    store(b + (0x6 << (m - 4)), v6); store(b + (0x7 << (m - 4)), v7);
    store(b + (0x8 << (m - 4)), v8); store(b + (0x9 << (m - 4)), v9);
    store(b + (0xA << (m - 4)), vA); store(b + (0xB << (m - 4)), vB);
    store(b + (0xC << (m - 4)), vC); store(b + (0xD << (m - 4)), vD);
    store(b + (0xE << (m - 4)), vE); store(b + (0xF << (m - 4)), vF);                
}

void fFFT4_x4()
{
    float theta = highest_freq * iX;

    vec2    w = iexp(theta); 
    vec2   rw = cmul(w, root16_1); 
    vec2   sw = cmr8(w);
    vec2  srw = cmul(w, root16_3); 
    vec2   iw = imul(w);
    vec2  irw = cmul(w, root16_5); 
    vec2  isw = cm83(w);
    vec2 isrw = cmul(w, root16_7);

    vec2   w2 = csqr(w);
    vec2  sw2 = cmr8(w2);
    vec2  iw2 = imul(w2);
    vec2 isw2 = cm83(w2);

    barrier();     

    vec2 u0, u1, u2, u3, u4, u5, u6, u7, u8, u9, uA, uB, uC, uD, uE, uF, 
         v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, vA, vB, vC, vD, vE, vF;

    u0 = Z[iX + (0x0 << (m - 4))]; u1 = Z[iX + (0x1 << (m - 4))]; u2 = Z[iX + (0x2 << (m - 4))]; u3 = Z[iX + (0x3 << (m - 4))]; 
    u4 = Z[iX + (0x4 << (m - 4))]; u5 = Z[iX + (0x5 << (m - 4))]; u6 = Z[iX + (0x6 << (m - 4))]; u7 = Z[iX + (0x7 << (m - 4))]; 
    u8 = Z[iX + (0x8 << (m - 4))]; u9 = Z[iX + (0x9 << (m - 4))]; uA = Z[iX + (0xA << (m - 4))]; uB = Z[iX + (0xB << (m - 4))]; 
    uC = Z[iX + (0xC << (m - 4))]; uD = Z[iX + (0xD << (m - 4))]; uE = Z[iX + (0xE << (m - 4))]; uF = Z[iX + (0xF << (m - 4))]; 

    u4 = cmul(  w2, u4); 
    u5 = cmul( sw2, u5); 
    u6 = cmul( iw2, u6); 
    u7 = cmul(isw2, u7);
    uC = cmul(  w2, uC); 
    uD = cmul( sw2, uD); 
    uE = cmul( iw2, uE); 
    uF = cmul(isw2, uF);

    butterfly_16(u0, u4, u1, u5, u2, u6, u3, u7, u8, uC, u9, uD, uA, uE, uB, uF,
                 v0, v4, v1, v5, v2, v6, v3, v7, v8, vC, v9, vD, vA, vE, vB, vF);

    v8 = cmul(   w, v8); 
    v9 = cmul(  rw, v9); 
    vA = cmul(  sw, vA);
    vB = cmul( srw, vB); 
    vC = cmul(  iw, vC);
    vD = cmul( irw, vD); 
    vE = cmul( isw, vE);
    vF = cmul(isrw, vF);

    butterfly_16(v0, v8, v1, v9, v2, vA, v3, vB, v4, vC, v5, vD, v6, vE, v7, vF,
                 u0, u8, u1, u9, u2, uA, u3, uB, u4, uC, u5, uD, u6, uE, u7, uF);

    int b = (iY << m) + iX;

    store(b + (0x0 << (m - 4)), u0); store(b + (0x1 << (m - 4)), u1);
    store(b + (0x2 << (m - 4)), u2); store(b + (0x3 << (m - 4)), u3);
    store(b + (0x4 << (m - 4)), u4); store(b + (0x5 << (m - 4)), u5);
    store(b + (0x6 << (m - 4)), u6); store(b + (0x7 << (m - 4)), u7);
    store(b + (0x8 << (m - 4)), u8); store(b + (0x9 << (m - 4)), u9);
    store(b + (0xA << (m - 4)), uA); store(b + (0xB << (m - 4)), uB);
    store(b + (0xC << (m - 4)), uC); store(b + (0xD << (m - 4)), uD);
    store(b + (0xE << (m - 4)), uE); store(b + (0xF << (m - 4)), uF);                
}

void fFFT2_x8()
{
    float theta = highest_freq * iX;

    vec2    w = iexp(theta); 
    vec2   rw = cmul(w, root16_1); 
    vec2   sw = cmr8(w);
    vec2  srw = cmul(w, root16_3); 
    vec2   iw = imul(w);
    vec2  irw = cmul(w, root16_5); 
    vec2  isw = imul(sw);
    vec2 isrw = cmul(w, root16_7);

    barrier();     

    vec2 u0, u1, u2, u3, u4, u5, u6, u7, u8, u9, uA, uB, uC, uD, uE, uF, 
         v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, vA, vB, vC, vD, vE, vF;

    u0 = Z[iX + (0x0 << (m - 4))]; u1 = Z[iX + (0x1 << (m - 4))]; u2 = Z[iX + (0x2 << (m - 4))]; u3 = Z[iX + (0x3 << (m - 4))]; 
    u4 = Z[iX + (0x4 << (m - 4))]; u5 = Z[iX + (0x5 << (m - 4))]; u6 = Z[iX + (0x6 << (m - 4))]; u7 = Z[iX + (0x7 << (m - 4))]; 
    u8 = Z[iX + (0x8 << (m - 4))]; u9 = Z[iX + (0x9 << (m - 4))]; uA = Z[iX + (0xA << (m - 4))]; uB = Z[iX + (0xB << (m - 4))]; 
    uC = Z[iX + (0xC << (m - 4))]; uD = Z[iX + (0xD << (m - 4))]; uE = Z[iX + (0xE << (m - 4))]; uF = Z[iX + (0xF << (m - 4))]; 

    u8 = cmul(   w, u8); 
    u9 = cmul(  rw, u9); 
    uA = cmul(  sw, uA);
    uB = cmul( srw, uB); 
    uC = cmul(  iw, uC);
    uD = cmul( irw, uD); 
    uE = cmul( isw, uE);
    uF = cmul(isrw, uF);

    butterfly_16(u0, u8, u1, u9, u2, uA, u3, uB, u4, uC, u5, uD, u6, uE, u7, uF, 
                 v0, v8, v1, v9, v2, vA, v3, vB, v4, vC, v5, vD, v6, vE, v7, vF);

    int b = (iY << m) + iX;

    store(b + (0x0 << (m - 4)), v0); store(b + (0x1 << (m - 4)), v1);
    store(b + (0x2 << (m - 4)), v2); store(b + (0x3 << (m - 4)), v3);
    store(b + (0x4 << (m - 4)), v4); store(b + (0x5 << (m - 4)), v5);
    store(b + (0x6 << (m - 4)), v6); store(b + (0x7 << (m - 4)), v7);
    store(b + (0x8 << (m - 4)), v8); store(b + (0x9 << (m - 4)), v9);
    store(b + (0xA << (m - 4)), vA); store(b + (0xB << (m - 4)), vB);
    store(b + (0xC << (m - 4)), vC); store(b + (0xD << (m - 4)), vD);
    store(b + (0xE << (m - 4)), vE); store(b + (0xF << (m - 4)), vF);                
}







void fFFT8_x1()
{
    float theta = highest_freq * iX;

    vec2   w = iexp(theta); 
    vec2  sw = cmr8(w);
    vec2  iw = imul(w);
    vec2 isw = cm83(w);

    vec2  w2 = csqr(w);
    vec2 iw2 = imul(w2);

    vec2  w4 = csqr(w2);

    barrier();

    vec2 u0, u1, u2, u3, u4, u5, u6, u7, 
         v0, v1, v2, v3, v4, v5, v6, v7; 

    u0 = Z[iX + (0 << (m - 3))]; u1 = Z[iX + (1 << (m - 3))]; u2 = Z[iX + (2 << (m - 3))]; u3 = Z[iX + (3 << (m - 3))];
    u4 = Z[iX + (4 << (m - 3))]; u5 = Z[iX + (5 << (m - 3))]; u6 = Z[iX + (6 << (m - 3))]; u7 = Z[iX + (7 << (m - 3))];

    u1 = cmul(w4, u1);
    u3 = cmul(w4, u3); 
    u5 = cmul(w4, u5);
    u7 = cmul(w4, u7); 

    butterfly_8(u0, u1, u2, u3, u4, u5, u6, u7, 
                v0, v1, v2, v3, v4, v5, v6, v7);

    v2 = cmul( w2, v2);
    v3 = cmul(iw2, v3);
    v6 = cmul( w2, v6);
    v7 = cmul(iw2, v7);

    butterfly_8(v0, v2, v1, v3, v4, v6, v5, v7,
                u0, u2, u1, u3, u4, u6, u5, u7);

    u4 = cmul(  w, u4); 
    u5 = cmul( sw, u5);
    u6 = cmul( iw, u6); 
    u7 = cmul(isw, u7);

    butterfly_8(u0, u4, u1, u5, u2, u6, u3, u7,
                v0, v4, v1, v5, v2, v6, v3, v7);

    int b = (iY << m) + iX;

    store(b + (0x0 << (m - 3)), v0); store(b + (0x1 << (m - 3)), v1);
    store(b + (0x2 << (m - 3)), v2); store(b + (0x3 << (m - 3)), v3);
    store(b + (0x4 << (m - 3)), v4); store(b + (0x5 << (m - 3)), v5);
    store(b + (0x6 << (m - 3)), v6); store(b + (0x7 << (m - 3)), v7);
}

void fFFT4_x1()
{
    float theta = highest_freq * iX;

    vec2  w = iexp(theta);
    vec2 iw = imul(w);
    vec2 w2 = csqr(w);

    barrier();

    vec2 u0, u1, u2, u3, 
         v0, v1, v2, v3; 

    u0 = Z[iX + (0 << (m - 2))]; u1 = Z[iX + (1 << (m - 2))]; u2 = Z[iX + (2 << (m - 2))]; u3 = Z[iX + (3 << (m - 2))];

    u1 = cmul(w2, u1);
    u3 = cmul(w2, u3); 

    butterfly_4(u0, u1, u2, u3, 
                v0, v1, v2, v3);

    v2 = cmul( w, v2);
    v3 = cmul(iw, v3);

    butterfly_4(v0, v2, v1, v3,
                u0, u2, u1, u3);

    int b = (iY << m) + iX;

    store(b + (0 << (m - 2)), u0); store(b + (1 << (m - 2)), u1);
    store(b + (2 << (m - 2)), u2); store(b + (3 << (m - 2)), u3);
}

void fFFT2_x1()
{
    vec2 w = iexp(highest_freq * iX); 
    barrier();     

    vec2 u0 = Z[iX + (0 << (m - 1))]; 
    vec2 u1 = Z[iX + (1 << (m - 1))];
    u1 = cmul(w, u1);

    int b = (iY << m) + iX;
    store(b + (0 << (m - 1)), u0 + u1);
    store(b + (1 << (m - 1)), u0 - u1);
}


//========================================================================================================================================================================================================================
// FFT512 as FFT8 + FFT8 + FFT8 :: works
//========================================================================================================================================================================================================================
void FFT512_8x1_8x1_8x1()
{
    iFFT8_x1();
    mFFT8_x1(8, pi / 32);
    fFFT8_x1();
}

//========================================================================================================================================================================================================================
// FFT256 as FFT4 + FFT4 + FFT4 + FFT4 :: works
//========================================================================================================================================================================================================================
void FFT256_4x1_4x1_4x1_4x1()
{
    iFFT4_x1();
    mFFT4_x1(4, pi / 8);
    mFFT4_x1(16, pi / 32);
    fFFT4_x1();
}

//========================================================================================================================================================================================================================
// FFT256 as FFT16 + FFT16 :: works
//========================================================================================================================================================================================================================
void FFT256_16x1_16x1()
{
    iFFT16_x1();
    fFFT16_x1();
}

//========================================================================================================================================================================================================================
// FFT1024 as FFT16 + FFT16 + FFT4x4 :: works
//========================================================================================================================================================================================================================
void FFT1024_16x1_16x1_4x4()
{
    iFFT16_x1();
    mFFT16_x1(16, pi / 128);
    fFFT4_x4();
}

//========================================================================================================================================================================================================================
// FFT512 as FFT16 + FFT16 + FFT2x8 :: works
//========================================================================================================================================================================================================================
void FFT512_16x1_16x1_2x8()
{
    iFFT16_x1();
    mFFT16_x1(16, pi / 128);
    fFFT2_x8();
}

//========================================================================================================================================================================================================================
// FFT2048 as FFT16 + FFT16 + FFT2x8 :: works
//========================================================================================================================================================================================================================
void FFT2048_16x1_16x1_8x2()
{
    iFFT16_x1();
    mFFT16_x1(16, pi / 128);
    fFFT8_x2();
}

//========================================================================================================================================================================================================================
// FFT4096 as FFT16 + FFT16 + FFT16 :: works
//========================================================================================================================================================================================================================
void FFT4096_16x1_16x1_16x1()
{
    iFFT16_x1();
    mFFT16_x1(16, pi / 128);
    fFFT16_x1();
}


void main()
{
    //FFT512_8x1_8x1_8x1();
    //FFT256_4x1_4x1_4x1_4x1();
    //FFT256_16x1_16x1();
    //FFT1024_16x1_16x1_4x4();
    //FFT512_16x1_16x1_2x8();
    FFT2048_16x1_16x1_8x2();
    //FFT4096_16x1_16x1_16x1();
}