#version 330

uniform sampler2D noise_texture;
in vec2 v_texCoord2D;

// one texel and half texel sizes -- corresponding to 256 x 256 texture size
const float ONE = 1.0 / 256.0;
const float ONEHALF = 1.0 / 512.0;

// 2D simplex noise
float snoise(vec2 P)
{
                                                                                            // Skew and unskew factors for 2D
    const float F2 = 0.366025403784;                                                        // (sqrt(3) - 1) / 2
    const float G2 = 0.211324865405;                                                        // (3 - sqrt(3)) / 6
    const float H2 = 0.577350269190;                                                        // 1 / sqrt(3)

                                                                                            // Skew the (x,y) space to determine which cell of 2 simplices we're in
    float s = (P.x + P.y) * F2;                                                             // Hairy factor for 2D skewing
    vec2 Pi = floor(P + s);
    float t = (Pi.x + Pi.y) * G2;                                                           // Hairy factor for unskewing
    vec2 P0 = Pi - t;                                                                       // Unskew the cell origin back to (x,y) space
    Pi = Pi * ONE + ONEHALF;                                                                // Integer part, scaled and offset for texture lookup

    vec2 Pf0 = P - P0;                                                                      // The (x, y) distances from the cell origin. For the 2D case, the simplex shape is an equilateral triangle.
                                                                                            // Find out whether we are above or below the x=y diagonal to determine which of the two triangles we're in.
    vec2 o1 = (Pf0.x > Pf0.y) ? vec2(1.0, 0.0): vec2(0.0, 1.0);
    
    vec2 grad0 = texture(noise_texture, Pi).rg * 4.0 - 1.0;                                   // Gradient at simplex origin
    vec2 grad1 = texture(noise_texture, Pi + o1*ONE).rg * 4.0 - 1.0;                          // Gradient at middle corner
    vec2 grad2 = texture(noise_texture, Pi + vec2(ONE, ONE)).rg * 4.0 - 1.0;                  // Gradient at last corner
    
    vec2 Pf1 = Pf0 - o1 + G2;
    vec2 Pf2 = Pf0 - vec2(H2);
    
    vec3 n012 = 0.5 - vec3(dot(Pf0, Pf0), dot(Pf1, Pf1), dot(Pf2, Pf2));                    // Perform all three blend kernel computations in parallel on vec3 data
    n012 = max(n012, 0.0);
    n012 *= n012;
    n012 *= n012 * vec3(dot(grad0, Pf0), dot(grad1, Pf1), dot(grad2, Pf2));
    
    return 70.0 * (n012.x + n012.y + n012.z);                                               // Sum up and scale the result to cover the range [-1,1]
}

out vec4 FragmentColor;

void main()
{
    float n = snoise(v_texCoord2D);
    FragmentColor = vec4(0.5 + 0.5 * vec3(n, n, n), 1.0);
}
