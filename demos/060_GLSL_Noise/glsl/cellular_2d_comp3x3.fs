#version 330

in vec2 v_texCoord2D;

//==============================================================================================================================================================
// Permutation polynomial: (34xx + x) mod 289
//==============================================================================================================================================================
vec3 permute(vec3 x) 
    { return mod((34.0f * x + 1.0f) * x, 289.0f); }

//==============================================================================================================================================================
//  Plain cellular noise : standard 3x3 search window for good F1 and F2 values
//==============================================================================================================================================================
vec2 cellular(vec2 P) 
{
    const float K = 1.0f / 7.0f;
    const float Ko = 3.0f / 7.0f;
    const float jitter = 1.0f;                                              // Less gives more regular pattern
    
    vec2 Pi = mod(floor(P), 289.0f);
    vec2 Pf = fract(P);
    vec3 oi = vec3(-1.0f, 0.0f, 1.0f);
    vec3 of = vec3(-0.5f, 0.5f, 1.5f);
    vec3 px = permute(Pi.x + oi);
    vec3 p = permute(px.x + Pi.y + oi);                                     // p11, p12, p13
    vec3 ox = fract(p * K) - Ko;
    vec3 oy = mod(floor(p * K), 7.0f) * K - Ko;
    vec3 dx = Pf.x + 0.5 + jitter * ox;
    vec3 dy = Pf.y - of + jitter * oy;
    vec3 d1 = dx * dx + dy * dy;                                            // d11, d12 and d13, squared
    p = permute(px.y + Pi.y + oi);                                          // p21, p22, p23
    ox = fract(p * K) - Ko;
    oy = mod(floor(p * K), 7.0f) * K - Ko;
    dx = Pf.x - 0.5 + jitter * ox;
    dy = Pf.y - of + jitter * oy;
    vec3 d2 = dx * dx + dy * dy;                                            // d21, d22 and d23, squared
    p = permute(px.z + Pi.y + oi);                                          // p31, p32, p33
    ox = fract(p * K) - Ko;
    oy = mod(floor(p * K), 7.0f) * K - Ko;
    dx = Pf.x - 1.5 + jitter * ox;
    dy = Pf.y - of + jitter * oy;
    vec3 d3 = dx * dx + dy * dy;                                            // d31, d32 and d33, squared

    vec3 d1a = min(d1, d2);                                                 // Sort out the two smallest distances (F1, F2)
    d2 = max(d1, d2);                                                       // Swap to keep candidates for F2
    d2 = min(d2, d3);                                                       // neither F1 nor F2 are now in d3
    d1 = min(d1a, d2);                                                      // F1 is now in d1
    d2 = max(d1a, d2);                                                      // Swap to keep candidates for F2
    d1.xy = (d1.x < d1.y) ? d1.xy : d1.yx;                                  // Swap if smaller
    d1.xz = (d1.x < d1.z) ? d1.xz : d1.zx;                                  // F1 is in d1.x
    d1.yz = min(d1.yz, d2.yz);                                              // F2 is now not in d2.yz
    d1.y = min(d1.y, d1.z);                                                 // nor in d1.z
    d1.y = min(d1.y, d2.x);                                                 // F2 is in d1.y, we're done.
    return sqrt(d1.xy);
}



out vec4 FragmentColor;

void main()
{
    vec2 F = cellular(v_texCoord2D);
    FragmentColor = vec4(F.x, F.y, 0.0, 1.0);
}
