#version 330

uniform sampler2D permTexture;
in vec3 v_texCoord3D;

// one texel and half texel sizes -- corresponding to 256 x 256 texture size
const float ONE = 1.0 / 256.0;
const float ONEHALF = 1.0 / 512.0;

// Efficient simplex indexing functions by Bill Licea-Kane, ATI. Thanks!
void simplex(const in vec3 P, out vec3 offset1, out vec3 offset2)
{
    vec3 offset0;

    vec2 isX = step(P.yz, P.xx);                                                                // P.x >= P.y ? 1.0 : 0.0;  P.x >= P.z ? 1.0 : 0.0;
    offset0.x  = dot(isX, vec2(1.0));                                                           // Accumulate all P.x >= other channels in offset.x
    offset0.yz = 1.0 - isX;                                                                     // Accumulate all P.x <  other channels in offset.yz
                                                                                                
    float isY = step(P.z, P.y);                                                                 // P.y >= P.z ? 1.0 : 0.0;
    offset0.y += isY;                                                                           // Accumulate P.y >= P.z in offset.y
    offset0.z += 1.0 - isY;                                                                     // Accumulate P.y <  P.z in offset.z
                                                                                                
                                                                                                // offset0 now contains the unique values 0,1,2 in each channel
                                                                                                // 2 for the channel greater than other channels
                                                                                                // 1 for the channel that is less than one but greater than another
                                                                                                // 0 for the channel less than other channels
                                                                                                // Equality ties are broken in favor of first x, then y
                                                                                                // (z always loses ties)

    offset2 = clamp(offset0, 0.0, 1.0);                                                         // offset2 contains 1 in each channel that was 1 or 2
    offset1 = clamp(offset0 - 1.0, 0.0, 1.0);                                                   // offset1 contains 1 in the single channel that was 1
}

// 3D simplex noise

float snoise(vec3 P)
{
    const float F3 = 1.0 / 3.0;                                                                 // The skewing and unskewing factors are much simpler for the 3D case
    const float G3 = 1.0 / 6.0;                                                                 
    const float H3 = 0.5;                                                                       // This is 1.0-3.0*G3, to remove a constant multiplication later
                                                                                                
                                                                                                // Skew the (x,y,z) space to determine which cell of 6 simplices we're in
    float s = (P.x + P.y + P.z) * F3;                                                           // Factor for 3D skewing
    vec3 Pi = floor(P + s);                                                                     
    float t = (Pi.x + Pi.y + Pi.z) * G3;                                                        
    vec3 P0 = Pi - t;                                                                           // Unskew the cell origin back to (x,y,z) space
    Pi = Pi * ONE + ONEHALF;                                                                    // Integer part, scaled and offset for texture lookup
                                                                                                
    vec3 Pf0 = P - P0;                                                                          // The (x, y) distances from the cell origin
                                                                                                // For the 3D case, the simplex shape is a slightly irregular tetrahedron.
                                                                                                // To find out which of the six possible tetrahedra we're in, we need to
                                                                                                // determine the magnitude ordering of x, y and z components of Pf0.
    vec3 o1;
    vec3 o2;
    simplex(Pf0, o1, o2);
  
    float perm0 = texture(noise_texture, Pi.xy).a;                                                // Gradient at simplex origin
    vec3  grad0 = texture(noise_texture, vec2(perm0, Pi.z)).rgb * 4.0 - 1.0;
    float perm1 = texture(noise_texture, Pi.xy + o1.xy*ONE).a;                                    // Gradient at second corner
    vec3  grad1 = texture(noise_texture, vec2(perm1, Pi.z + o1.z*ONE)).rgb * 4.0 - 1.0;
    float perm2 = texture(noise_texture, Pi.xy + o2.xy*ONE).a;                                    // Gradient at third corner
    vec3  grad2 = texture(noise_texture, vec2(perm2, Pi.z + o2.z*ONE)).rgb * 4.0 - 1.0;
    float perm3 = texture(noise_texture, Pi.xy + vec2(ONE, ONE)).a;                               // Gradient at last corner
    vec3  grad3 = texture(noise_texture, vec2(perm3, Pi.z + ONE)).rgb * 4.0 - 1.0;

    vec3 Pf1 = Pf0 - o1 + G3;
    vec3 Pf2 = Pf0 - o2 + 2.0 * G3;
    vec3 Pf3 = Pf0 - vec3(H3);
  
    vec4 n0123 = 0.6 - vec4(dot(Pf0, Pf0), dot(Pf1, Pf1), dot(Pf2, Pf2), dot(Pf3, Pf3));        // Perform all four blend kernel computations in parallel on vec4 data
    n0123 = max(n0123, 0.0);
    n0123 *= n0123;
    n0123 *= n0123 * vec4(dot(grad0, Pf0), dot(grad1, Pf1), dot(grad2, Pf2), dot(grad3, Pf3));
  
    return 32.0 * (n0123.x + n0123.y + n0123.z + n0123.w);                                      // Sum up and scale the result to cover the range [-1,1]
}

out vec4 FragmentColor;


void main()
{
    float n = snoise(v_texCoord3D);
    FragmentColor = vec4(0.5 + 0.5 * vec3(n, n, n), 1.0);
}
