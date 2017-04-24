#version 330

uniform sampler2D permTexture;
uniform sampler2D gradTexture;
in vec4 v_texCoord4D;

// one texel and half texel sizes -- corresponding to 256 x 256 texture size
const float ONE = 1.0 / 256.0;
const float ONEHALF = 1.0 / 512.0;

float fade(float t)
    { return t * t * t * (t * (t * 6.0 - 15.0) + 10.0); }

// 4D classic noise. Slow, but very useful.
float noise(vec4 P)
{
    vec4 Pi = ONE * floor(P) + ONEHALF;                                                                 // Integer part, scaled so +1 moves one texel and offset 1 / 2 texel to sample texel centers
    vec4 Pf = fract(P);                                                                                 // Fractional part for interpolation
    
    
    float perm00xy = texture(noise_texture, Pi.xy).a;                                                     // "n0000" is the noise contribution from (x = 0, y = 0, z = 0, w = 0), and so on
    float perm00zw = texture(noise_texture, Pi.zw).a;
    vec4 grad0000 = texture(gradTexture, vec2(perm00xy, perm00zw)).rgba * 4.0 -1.0;
    float n0000 = dot(grad0000, Pf);
    
    float perm01zw = texture(noise_texture, Pi.zw  + vec2(0.0, ONE)).a ;
    vec4  grad0001 = texture(gradTexture, vec2(perm00xy, perm01zw)).rgba * 4.0 - 1.0;
    float n0001 = dot(grad0001, Pf - vec4(0.0, 0.0, 0.0, 1.0));
    
    float perm10zw = texture(noise_texture, Pi.zw  + vec2(ONE, 0.0)).a ;
    vec4  grad0010 = texture(gradTexture, vec2(perm00xy, perm10zw)).rgba * 4.0 - 1.0;
    float n0010 = dot(grad0010, Pf - vec4(0.0, 0.0, 1.0, 0.0));
    
    float perm11zw = texture(noise_texture, Pi.zw  + vec2(ONE, ONE)).a ;
    vec4  grad0011 = texture(gradTexture, vec2(perm00xy, perm11zw)).rgba * 4.0 - 1.0;
    float n0011 = dot(grad0011, Pf - vec4(0.0, 0.0, 1.0, 1.0));
    
    float perm01xy = texture(noise_texture, Pi.xy + vec2(0.0, ONE)).a ;
    vec4  grad0100 = texture(gradTexture, vec2(perm01xy, perm00zw)).rgba * 4.0 - 1.0;
    float n0100 = dot(grad0100, Pf - vec4(0.0, 1.0, 0.0, 0.0));
    
    vec4  grad0101 = texture(gradTexture, vec2(perm01xy, perm01zw)).rgba * 4.0 - 1.0;
    float n0101 = dot(grad0101, Pf - vec4(0.0, 1.0, 0.0, 1.0));
    
    vec4  grad0110 = texture(gradTexture, vec2(perm01xy, perm10zw)).rgba * 4.0 - 1.0;
    float n0110 = dot(grad0110, Pf - vec4(0.0, 1.0, 1.0, 0.0));
    
    vec4  grad0111 = texture(gradTexture, vec2(perm01xy, perm11zw)).rgba * 4.0 - 1.0;
    float n0111 = dot(grad0111, Pf - vec4(0.0, 1.0, 1.0, 1.0));
    
    float perm10xy = texture(noise_texture, Pi.xy + vec2(ONE, 0.0)).a ;
    vec4  grad1000 = texture(gradTexture, vec2(perm10xy, perm00zw)).rgba * 4.0 - 1.0;
    float n1000 = dot(grad1000, Pf - vec4(1.0, 0.0, 0.0, 0.0));
    
    vec4  grad1001 = texture(gradTexture, vec2(perm10xy, perm01zw)).rgba * 4.0 - 1.0;
    float n1001 = dot(grad1001, Pf - vec4(1.0, 0.0, 0.0, 1.0));
    
    vec4  grad1010 = texture(gradTexture, vec2(perm10xy, perm10zw)).rgba * 4.0 - 1.0;
    float n1010 = dot(grad1010, Pf - vec4(1.0, 0.0, 1.0, 0.0));
    
    vec4  grad1011 = texture(gradTexture, vec2(perm10xy, perm11zw)).rgba * 4.0 - 1.0;
    float n1011 = dot(grad1011, Pf - vec4(1.0, 0.0, 1.0, 1.0));
    
    float perm11xy = texture(noise_texture, Pi.xy + vec2(ONE, ONE)).a ;
    vec4  grad1100 = texture(gradTexture, vec2(perm11xy, perm00zw)).rgba * 4.0 - 1.0;
    float n1100 = dot(grad1100, Pf - vec4(1.0, 1.0, 0.0, 0.0));
    
    vec4  grad1101 = texture(gradTexture, vec2(perm11xy, perm01zw)).rgba * 4.0 - 1.0;
    float n1101 = dot(grad1101, Pf - vec4(1.0, 1.0, 0.0, 1.0));
    
    vec4  grad1110 = texture(gradTexture, vec2(perm11xy, perm10zw)).rgba * 4.0 - 1.0;
    float n1110 = dot(grad1110, Pf - vec4(1.0, 1.0, 1.0, 0.0));
    
    vec4  grad1111 = texture(gradTexture, vec2(perm11xy, perm11zw)).rgba * 4.0 - 1.0;
    float n1111 = dot(grad1111, Pf - vec4(1.0, 1.0, 1.0, 1.0));
    
    float fadex = fade(Pf.x);                                                                           // Blend contributions along x
    vec4 n_x0 = mix(vec4(n0000, n0001, n0010, n0011), vec4(n1000, n1001, n1010, n1011), fadex);
    vec4 n_x1 = mix(vec4(n0100, n0101, n0110, n0111), vec4(n1100, n1101, n1110, n1111), fadex);
    vec4 n_xy = mix(n_x0, n_x1, fade(Pf.y));                                                            // Blend contributions along y
    vec2 n_xyz = mix(n_xy.xy, n_xy.zw, fade(Pf.z));                                                     // Blend contributions along z
    float n_xyzw = mix(n_xyz.x, n_xyz.y, fade(Pf.w));                                                   // Blend contributions along w
    
    return n_xyzw;                                                                                      // return the final noise value
}

out vec4 FragmentColor;

void main()
{
    float n = noise(v_texCoord4D);
    FragmentColor = vec4(0.5 + 0.5 * vec3(n, n, n), 1.0);
}
