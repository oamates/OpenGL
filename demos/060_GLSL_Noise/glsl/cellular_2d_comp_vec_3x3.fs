#version 330

in vec2 v_texCoord2D;

vec3 permute(vec3 x) 
    { return mod((34.0f * x + 1.0f) * x, 289.0f); }

//==============================================================================================================================================================
//  Plain cellular noise returning the 2D vectors to each of the two closest points in a vec4
//==============================================================================================================================================================
vec2 cellular(vec2 P, out vec4 d1d2) 
{
    const float K = 0.142857142857f;                                        // 1.0f / 7.0f
    const float Ko = 0.428571428571f;                                       // 3.0f / 7.0f
    const float jitter = 1.0f;                                              // Less gives more regular pattern

    // Standard 3x3 search window for good F1 and F2 values.
    vec2 Pi = mod(floor(P), 289.0f);
    vec2 Pf = fract(P);
    vec3 oi = vec3(-1.0f, 0.0f, 1.0f);
    vec3 of = vec3(-0.5f, 0.5f, 1.5f);
    vec3 px = permute(Pi.x + oi);
    vec3 p = permute(px.x + Pi.y + oi);                                     // p11, p12, p13
    vec3 ox = fract(p * K) - Ko;
    vec3 oy = mod(floor(p * K), 7.0f) * K - Ko;
    vec3 d1x = Pf.x + 0.5f + jitter * ox;                                      
    vec3 d1y = Pf.y - of + jitter * oy;
    vec3 d1 = d1x * d1x + d1y * d1y;                                        // d11, d12 and d13, squared
    p = permute(px.y + Pi.y + oi);                                          // p21, p22, p23
    ox = fract(p * K) - Ko;
    oy = mod(floor(p * K), 7.0f) * K - Ko;
    vec3 d2x = Pf.x - 0.5 + jitter * ox;
    vec3 d2y = Pf.y - of + jitter * oy;
    vec3 d2 = d2x * d2x + d2y * d2y;                                        // d21, d22 and d23, squared
    p = permute(px.z + Pi.y + oi);                                          // p31, p32, p33
    ox = fract(p * K) - Ko;
    oy = mod(floor(p * K), 7.0) * K - Ko;
    vec3 d3x = Pf.x - 1.5 + jitter * ox;
    vec3 d3y = Pf.y - of + jitter * oy;
    vec3 d3 = d3x * d3x + d3y * d3y;                                        // d31, d32 and d33, squared

    // Sort out the two smallest distances (F1, F2). While also swapping dx and dy accordingly
    vec3 comp3 = step(d2, d1);
    vec3 d1a = mix(d1, d2, comp3);
    vec3 d1xa = mix(d1x, d2x, comp3);
    vec3 d1ya = mix(d1y, d2y, comp3);
    d2 = mix(d2, d1, comp3);                                                // Swap to keep candidates for F2
    d2x = mix(d2x, d1x, comp3);
    d2y = mix(d2y, d1y, comp3);
    comp3 = step(d3, d2);
    d2 = mix(d2, d3, comp3);                                                // neither F1 nor F2 are now in d3
    d2x = mix(d2x, d3x, comp3);
    d2y = mix(d2y, d3y, comp3);
    comp3 = step(d2, d1a);
    d1 = mix(d1a, d2, comp3);                                               // F1 is now in d1
    d1x = mix(d1xa, d2x, comp3);
    d1y = mix(d1ya, d2y, comp3);
    d2 = mix(d2, d1a, comp3);                                               // Swap to keep candidates for F2
    d2x = mix(d2x, d1xa, comp3);
    d2y = mix(d2y, d1ya, comp3);
    float comp1 = step(d1.y, d1.x);
    d1.xy = mix(d1.xy, d1.yx, comp1);                                       // Swap if smaller
    d1x.xy = mix(d1x.xy, d1x.yx, comp1);
    d1y.xy = mix(d1y.xy, d1y.yx, comp1);
    comp1 = step(d1.z, d1.x);
    d1.xz = mix(d1.xz, d1.zx, comp1);                                       // F1 is in d1.x
    d1x.xz = mix(d1x.xz, d1x.zx, comp1);
    d1y.xz = mix(d1y.xz, d1y.zx, comp1);
    vec2 comp2 = step(d2.yz, d1.yz);
    d1.yz = mix(d1.yz, d2.yz, comp2);                                       // F2 is now not in d2.yz
    d1x.yz = mix(d1x.yz, d2x.yz, comp2);
    d1y.yz = mix(d1y.yz, d2y.yz, comp2);
    comp1 = step(d1.z, d1.y);
    d1.y = mix(d1.y, d1.z, comp1);                                          // nor in  d1.z
    d1x.y = mix(d1x.y, d1x.z, comp1);
    d1y.y = mix(d1y.y, d1y.z, comp1);
    comp1 = step(d2.x, d1.y);
    d1.y = mix(d1.y, d2.x, comp1);                                          // F2 is in d1.y, we're done.
    d1x.y = mix(d1x.y, d2x.x, comp1);
    d1y.y = mix(d1y.y, d2y.x, comp1);
    d1d2 = vec4(d1x.x, d1y.x, d1x.y, d1y.y);
    return sqrt(d1.xy);
}


out vec4 FragmentColor;

void main()
{
    vec4 d1d2;
    vec2 F = cellular(v_texCoord2D, d1d2);
    FragmentColor = vec4(F.x, F.y, 0.0, 1.0);
}
