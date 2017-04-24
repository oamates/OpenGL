#version 330

uniform sampler2D permTexture;
uniform sampler2D gradTexture;
in vec4 v_texCoord4D;

// one texel and half texel sizes -- corresponding to 256 x 256 texture size
const float ONE = 1.0 / 256.0;
const float ONEHALF = 1.0 / 512.0;

// Efficient simplex indexing functions by Bill Licea-Kane, ATI. Thanks!
void simplex( const in vec4 P, out vec4 offset1, out vec4 offset2, out vec4 offset3 )
{
    vec4 offset0;
    vec3 isX = step( P.yzw, P.xxx );                                                    // See comments in 3D simplex function
    offset0.x = dot( isX, vec3( 1.0 ) );
    offset0.yzw = 1.0 - isX;
    vec3 isYZ = step( P.zww, P.yyz );
    offset0.y += dot( isYZ.xy, vec2( 1.0 ) );
    offset0.zw += 1.0 - isYZ.xy;
    offset0.z += isYZ.z;
    offset0.w += 1.0 - isYZ.z;                                                          // offset0 now contains the unique values 0, 1, 2, 3 in each channel
    offset3 = clamp(offset0,       0.0, 1.0);
    offset2 = clamp(offset0 - 1.0, 0.0, 1.0);
    offset1 = clamp(offset0 - 2.0, 0.0, 1.0);
}


// 4D simplex noise, faster and better than 4d gradient noise
float snoise(vec4 P)
{
    const float F4 = 0.309016994375;                                                    // (sqrt(5) - 1) / 4
    const float G4 = 0.138196601125;                                                    // (5 - sqrt(5)) / 20
                                                                                        // Skew the (x, y, z, w) space to determine which cell of 24 simplices we're in
    float s = (P.x + P.y + P.z + P.w) * F4;                                             // Factor for 4D skewing
    vec4 Pi = floor(P + s);
    float t = (Pi.x + Pi.y + Pi.z + Pi.w) * G4;
    vec4 P0 = Pi - t;                                                                   // Unskew the cell origin back to (x,y,z,w) space
    Pi = Pi * ONE + ONEHALF;                                                            // Integer part, scaled and offset for texture lookup
    
    vec4 Pf0 = P - P0;                                                                  // The (x, y) distances from the cell origin
    
    vec4 o1;                                                                            // To find out which of the 24 possible simplices we're in, we need to determine the magnitude ordering of x, y, z and w components of Pf0.
    vec4 o2;
    vec4 o3;
    simplex(Pf0, o1, o2, o3);  
    
    float perm0xy = texture(noise_texture, Pi.xy).a;                                      // Noise contribution from simplex origin
    float perm0zw = texture(noise_texture, Pi.zw).a;
    vec4  grad0   = texture(gradTexture, vec2(perm0xy, perm0zw)).rgba * 4.0 - 1.0;
    float t0 = 0.6 - dot(Pf0, Pf0);
    float n0;
    if (t0 < 0.0)
        n0 = 0.0;
    else
    {
        t0 *= t0;
        n0 = t0 * t0 * dot(grad0, Pf0);
    }
    
    vec4 Pf1 = Pf0 - o1 + G4;                                                           // Noise contribution from second corner
    o1 = o1 * ONE;
    float perm1xy = texture(noise_texture, Pi.xy + o1.xy).a;
    float perm1zw = texture(noise_texture, Pi.zw + o1.zw).a;
    vec4  grad1   = texture(gradTexture, vec2(perm1xy, perm1zw)).rgba * 4.0 - 1.0;
    float t1 = 0.6 - dot(Pf1, Pf1);
    float n1;
    if (t1 < 0.0)
        n1 = 0.0;
    else
    {
        t1 *= t1;
        n1 = t1 * t1 * dot(grad1, Pf1);
    }
    
    vec4 Pf2 = Pf0 - o2 + 2.0 * G4;                                                     // Noise contribution from third corner
    o2 = o2 * ONE;
    float perm2xy = texture(noise_texture, Pi.xy + o2.xy).a;
    float perm2zw = texture(noise_texture, Pi.zw + o2.zw).a;
    vec4  grad2   = texture(gradTexture, vec2(perm2xy, perm2zw)).rgba * 4.0 - 1.0;
    float t2 = 0.6 - dot(Pf2, Pf2);
    float n2;
    if (t2 < 0.0) 
        n2 = 0.0;
    else
    {
        t2 *= t2;
        n2 = t2 * t2 * dot(grad2, Pf2);
    }
    
    vec4 Pf3 = Pf0 - o3 + 3.0 * G4;                                                     // Noise contribution from fourth corner
    o3 = o3 * ONE;
    float perm3xy = texture(noise_texture, Pi.xy + o3.xy).a;
    float perm3zw = texture(noise_texture, Pi.zw + o3.zw).a;
    vec4  grad3   = texture(gradTexture, vec2(perm3xy, perm3zw)).rgba * 4.0 - 1.0;
    float t3 = 0.6 - dot(Pf3, Pf3);
    float n3;
    if (t3 < 0.0)
        n3 = 0.0;
    else
    {
        t3 *= t3;
        n3 = t3 * t3 * dot(grad3, Pf3);
    }
    
    vec4 Pf4 = Pf0 - vec4(1.0 - 4.0 * G4);                                              // Noise contribution from last corner
    float perm4xy = texture(noise_texture, Pi.xy + vec2(ONE, ONE)).a;
    float perm4zw = texture(noise_texture, Pi.zw + vec2(ONE, ONE)).a;
    vec4  grad4   = texture(gradTexture, vec2(perm4xy, perm4zw)).rgba * 4.0 - 1.0;
    float t4 = 0.6 - dot(Pf4, Pf4);
    float n4;
    if (t4 < 0.0)
        n4 = 0.0;
    else
    {
        t4 *= t4;
        n4 = t4 * t4 * dot(grad4, Pf4);
    }
    
    return 27.0 * (n0 + n1 + n2 + n3 + n4);                                             // Sum up and scale the result to cover the range [-1,1]
}

out vec4 FragmentColor;

void main()
{
    float n = snoise(v_texCoord4D);
    FragmentColor = vec4(0.5 + 0.5 * vec3(n, n, n), 1.0);
}
