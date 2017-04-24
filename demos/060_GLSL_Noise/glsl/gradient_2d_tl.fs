#version 330

uniform sampler2D noise_texture;
in vec2 v_texCoord2D;

// one texel and half texel sizes -- corresponding to 256 x 256 texture size
const float ONE = 1.0 / 256.0;
const float ONEHALF = 1.0 / 512.0;

vec2 fade(vec2 t)
    { return t * t * t * (t * (t * 6.0 - 15.0) + 10.0); }

// 2D gradient noise.
float noise(vec2 P)
{
    vec2 Pi = ONE * floor(P) + ONEHALF;                                             // integral part, scaled and offset for texture lookup
    vec2 Pf = fract(P);                                                             // fractional part for interpolation

    vec2 grad00 = texture(noise_texture, Pi).rg * 4.0 - 1.0;                          // lower left corner
    float n00 = dot(grad00, Pf);
    vec2 grad10 = texture(noise_texture, Pi + vec2(ONE, 0.0)).rg * 4.0 - 1.0;         // lower right corner
    float n10 = dot(grad10, Pf - vec2(1.0, 0.0));
    vec2 grad01 = texture(noise_texture, Pi + vec2(0.0, ONE)).rg * 4.0 - 1.0;         // upper left corner
    float n01 = dot(grad01, Pf - vec2(0.0, 1.0));
    vec2 grad11 = texture(noise_texture, Pi + vec2(ONE, ONE)).rg * 4.0 - 1.0;         // upper right corner
    float n11 = dot(grad11, Pf - vec2(1.0, 1.0));

    vec2 fade_xy = fade(Pf);                                                        // blend factors
    vec2 n_x = mix(vec2(n00, n01), vec2(n10, n11), fade_xy.x);                      // blend along x
    float n_xy = mix(n_x.x, n_x.y, fade_xy.y);                                      // blend along y
    return n_xy;                                                                    // return the final noise value.
}

out vec4 FragmentColor;

void main()
{
    float n = noise(v_texCoord2D);
    FragmentColor = vec4(0.5 + 0.5 * vec3(n, n, n), 1.0);
}
