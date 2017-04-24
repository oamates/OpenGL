#version 330

uniform sampler2D permTexture;
in vec3 v_texCoord3D;

// one texel and half texel sizes -- corresponding to 256 x 256 texture size
const float TEXEL_SIZE = 1.0 / 256.0;
const float HALF_TEXEL = 1.0 / 512.0;

float fade(float t)
    { return t * t * t * (t * (t * 6.0 - 15.0) + 10.0); }

// gradient 3D noise
float noise(vec3 P)
{
    vec3 Pi = ONE * floor(P) + ONEHALF;                                                                     // Integer part, scaled so +1 moves one texel
                                                                                                            // and offset 1/2 texel to sample texel centers
    vec3 Pf = fract(P);                                                                                     // Fractional part for interpolation
    
    float perm00 = texture(noise_texture, Pi.xy).a;                                                           // Noise contributions from (x=0, y=0), z=0 and z=1
    vec3  grad000 = texture(noise_texture, vec2(perm00, Pi.z)).rgb * 4.0 - 1.0;
    float n000 = dot(grad000, Pf);
    vec3  grad001 = texture(noise_texture, vec2(perm00, Pi.z + TEXEL_SIZE)).rgb * 4.0 - 1.0;
    float n001 = dot(grad001, Pf - vec3(0.0, 0.0, 1.0));

    float perm01 = texture(noise_texture, Pi.xy + vec2(0.0, TEXEL_SIZE)).a;                                          // Noise contributions from (x=0, y=1), z=0 and z=1
    vec3  grad010 = texture(noise_texture, vec2(perm01, Pi.z)).rgb * 4.0 - 1.0;
    float n010 = dot(grad010, Pf - vec3(0.0, 1.0, 0.0));
    vec3  grad011 = texture(noise_texture, vec2(perm01, Pi.z + TEXEL_SIZE)).rgb * 4.0 - 1.0;
    float n011 = dot(grad011, Pf - vec3(0.0, 1.0, 1.0));

    float perm10 = texture(noise_texture, Pi.xy + vec2(ONE, 0.0)).a;                                          // Noise contributions from (x=1, y=0), z=0 and z=1
    vec3  grad100 = texture(noise_texture, vec2(perm10, Pi.z)).rgb * 4.0 - 1.0;
    float n100 = dot(grad100, Pf - vec3(1.0, 0.0, 0.0));
    vec3  grad101 = texture(noise_texture, vec2(perm10, Pi.z + TEXEL_SIZE)).rgb * 4.0 - 1.0;
    float n101 = dot(grad101, Pf - vec3(1.0, 0.0, 1.0));

    float perm11 = texture(noise_texture, Pi.xy + vec2(ONE, ONE)).a;                                          // Noise contributions from (x=1, y=1), z=0 and z=1
    vec3  grad110 = texture(noise_texture, vec2(perm11, Pi.z)).rgb * 4.0 - 1.0;
    float n110 = dot(grad110, Pf - vec3(1.0, 1.0, 0.0));
    vec3  grad111 = texture(noise_texture, vec2(perm11, Pi.z + TEXEL_SIZE)).rgb * 4.0 - 1.0;
    float n111 = dot(grad111, Pf - vec3(1.0, 1.0, 1.0));
    
    vec4 n_x = mix(vec4(n000, n001, n010, n011), vec4(n100, n101, n110, n111), fade(Pf.x));                 // Blend contributions along x
    vec2 n_xy = mix(n_x.xy, n_x.zw, fade(Pf.y));                                                            // Blend contributions along y
    float n_xyz = mix(n_xy.x, n_xy.y, fade(Pf.z));                                                          // Blend contributions along z

    return n_xyz;                                                                                           // return the final noise value
}

out vec4 FragmentColor;


void main()
{
    float n = noise(v_texCoord3D);
    FragmentColor = vec4(0.5 + 0.5 * vec3(n, n, n), 1.0);
}
