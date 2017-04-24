#version 330 core

uniform mat4 projection_view_matrix;
uniform sampler2D value_texture;

out float value;
out vec2 uv;
out vec3 position_ws;
out vec3 normal_ws;

const int n = 256;
const int N = 2 * n + 1;
const float inv_n = 1.0 / n;

const float TEXEL_SIZE = 1.0 / 256.0;
const float HALF_TEXEL = 1.0 / 512.0;

vec2 hermite5(vec2 x)    { return x * x * x * (10.0 + x * (6.0 * x - 15.0)); }
vec2 hermite5_d(vec2 x)  { vec2  q = x * (1.0 - x); return 30.0 * q * q; }

float vnoise(in vec2 x, out vec2 dF)
{
    vec2 p = floor(x);
    vec2 f = x - p;
    vec2 blend = hermite5(f);                                             
    vec2 blend_d = hermite5_d(f);                                         
    vec4 v = textureGather(value_texture, TEXEL_SIZE * p + HALF_TEXEL, 0);
    dF = mix(v.zx - v.ww, v.yy - v.xz, blend.yx) * blend_d;               
    vec2 blend_h = mix(v.wx, v.zy, blend.x);                              
    return mix(blend_h.x, blend_h.y, blend.y);                            
}

float fbm_vnoise(in vec2 x, out vec2 dF)
{
    const int level = 6;
    float frequency = 1.0;
    float amplitude = 0.5;
    float v = amplitude * vnoise(x, dF);
    for(int i = 0; i < level; ++i)
    {
        vec2 df;
        frequency *= 2.0;
        amplitude *= 0.5;    
        v += amplitude * vnoise(frequency * x, df);
        dF += df;
    }
    dF *= 0.5;
    return v;
}



void main()
{
    int iX = (gl_VertexID % N) - n;
    int iY = (gl_VertexID / N) - n;

    uv = inv_n * vec2(iX, iY);
    
    const float lattice_scale = 107.12917;
    const float terrain_scale = 5.127541;
    vec2 basepoint = lattice_scale * uv;
    vec2 dF;
    value = fbm_vnoise(terrain_scale * uv, dF);


    position_ws = vec3(basepoint.x, 5.0 * (value - 0.5f), basepoint.y);
    vec4 position_ws4 = vec4(position_ws, 1.0f);
    normal_ws = normalize(vec3(dF, -1.0f));

    gl_Position = projection_view_matrix * position_ws4;
}



