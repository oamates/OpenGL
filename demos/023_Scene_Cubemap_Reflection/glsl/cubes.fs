#version 430 core

in vec4 position;
in vec4 color;
in vec4 normal;
in vec4 tangent_x;
in vec4 tangent_y;
in vec4 view_direction;
in vec2 texture_coord;

const float two_pi = 6.283185307179586476925286766559f;
const float one_over_root2 = 0.70710678118654752440084436210485f;
const float one_over_root3 = 0.57735026918962576450914878050196f;

const float cube_size = 25.4;

const vec4 light_position[8] = 
{
    vec4(-cube_size, -cube_size, -cube_size, 1.0f),    
    vec4(-cube_size, -cube_size,  cube_size, 1.0f),    
    vec4(-cube_size,  cube_size, -cube_size, 1.0f),    
    vec4(-cube_size,  cube_size,  cube_size, 1.0f),    

    vec4( cube_size, -cube_size, -cube_size, 1.0f),    
    vec4( cube_size, -cube_size,  cube_size, 1.0f),    
    vec4( cube_size,  cube_size, -cube_size, 1.0f),    
    vec4( cube_size,  cube_size,  cube_size, 1.0f)
};


const vec4 light_color[8] = 
{
    vec4(one_over_root3, one_over_root3, one_over_root3, 1.0f),    
    vec4(one_over_root2, one_over_root2, 0.0f,           1.0f),    
    vec4(0.0f,           one_over_root2, one_over_root2, 1.0f),    
    vec4(one_over_root2, 0.0f,           one_over_root2, 1.0f),    
    vec4(1.0f,           0.0f,           0.0f,           1.0f),    
    vec4(0.0f,           1.0f,           0.0f,           1.0f),    
    vec4(0.0f,           0.0f,           1.0f,           1.0f),    
    vec4(one_over_root3, one_over_root3, one_over_root3, 1.0f)
};



uniform sampler2D texture_sampler;
uniform float global_time;

out vec4 fragment_color;

//=========================================================================
// NOISE 1
//=========================================================================

float rand(vec2 n) 
{ 
    return fract(sin(dot(n, vec2(12.9391f, 4.1417f))) * 43758.5453f);
};

float noise(vec2 p)
{
    vec2 ip = floor(p);
    vec2 u = fract(p);
    u = u * u * (3.0 - 2.0 * u);

    float res = mix(mix(rand(ip), rand(ip + vec2(1.0f, 0.0f)), u.x),
                    mix(rand(ip + vec2(0.0f, 1.0f)), rand(ip + vec2(1.0f, 1.0f)), u.x), u.y);
    return res * res;
}


vec4 procedural_texture_ambient(vec2 p)
{
    vec2 q = vec2(1.0f, 1.0f);

    float ns = (noise(p) + noise(p.yx) + noise(q - p) + noise(q - p.yx)) / 4.0f;
    return vec4(ns, ns, 0.0f, 1.0f);
};


vec4 procedural_texture_specular(vec2 p)
{
    return vec4(0.1, 0.1, 0.1, 1.0);
};

//=========================================================================
// CLASSIC PERLIN NOISE
//=========================================================================


vec2 fade(vec2 t) 
{
    return t*t*t*(t*(t*6.0 - 15.0) + 10.0);
};

vec4 permute(vec4 x)
{
    return mod(((x*34.0)+1.0)*x, 289.0);
};

float cnoise(vec2 P)
{
    vec4 Pi = floor(P.xyxy) + vec4(0.0, 0.0, 1.0, 1.0);
    vec4 Pf = fract(P.xyxy) - vec4(0.0, 0.0, 1.0, 1.0);
    Pi = mod(Pi, 289.0); // To avoid truncation effects in permutation
    vec4 ix = Pi.xzxz;
    vec4 iy = Pi.yyww;
    vec4 fx = Pf.xzxz;
    vec4 fy = Pf.yyww;
    vec4 i = permute(permute(ix) + iy);
    vec4 gx = 2.0 * fract(i * 0.0243902439) - 1.0; // 1/41 = 0.024...
    vec4 gy = abs(gx) - 0.5;
    vec4 tx = floor(gx + 0.5);
    gx = gx - tx;
    vec2 g00 = vec2(gx.x,gy.x);
    vec2 g10 = vec2(gx.y,gy.y);
    vec2 g01 = vec2(gx.z,gy.z);
    vec2 g11 = vec2(gx.w,gy.w);
    vec4 norm = 1.79284291400159 - 0.85373472095314 * vec4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11));
    g00 *= norm.x;
    g01 *= norm.y;
    g10 *= norm.z;
    g11 *= norm.w;
    float n00 = dot(g00, vec2(fx.x, fy.x));
    float n10 = dot(g10, vec2(fx.y, fy.y));
    float n01 = dot(g01, vec2(fx.z, fy.z));
    float n11 = dot(g11, vec2(fx.w, fy.w));
    vec2 fade_xy = fade(Pf.xy);
    vec2 n_x = mix(vec2(n00, n01), vec2(n10, n11), fade_xy.x);
    float n_xy = mix(n_x.x, n_x.y, fade_xy.y);
    return 2.3 * n_xy;
};

void main()
{

    vec3 normal_ms = vec3(texture(texture_sampler, texture_coord)) - vec3(0.5f, 0.5f, 0.0f);

    vec4 n = normalize(normal_ms.x * tangent_x + normal_ms.y * tangent_y + normal_ms.z * normal);   // normal vector to fragment
    vec4 v = normalize(view_direction);                                                         


    vec4 material_ambient_color = procedural_texture_ambient(texture_coord) + color;
    vec4 material_specular_color = procedural_texture_specular(texture_coord);

    fragment_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    vec4 specular_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 8; ++i)
    {
        float distance = length(light_position[i] - position);

        vec4 l = (light_position[i] - position) / distance;      // a direction to a source of light
        float lambert_cosine = dot(n, l);
        if (lambert_cosine < 0.0f) lambert_cosine *= lambert_cosine;
        vec4 r = reflect(l, n);
        float cos_alpha = dot(r, v);
        

        float ambient_distance_factor = 2 * clamp(250.0f / (distance * distance), 0.0f, 1.0f);
        float specular_distance_factor = 2 * clamp(150.0f / (distance * distance), 0.0f, 1.0f);
        fragment_color += material_ambient_color * light_color[i] * lambert_cosine * lambert_cosine * ambient_distance_factor;
        specular_color += material_specular_color * light_color[i] * pow(cos_alpha, 20) * specular_distance_factor;
    };
    fragment_color += specular_color;
//  fragment_color.w = 1.0f - 16.0f * texture_coord.x * (1.0f - texture_coord.x) * texture_coord.y * (1.0f - texture_coord.y);
//  fragment_color.w = 16 * clamp (texture_coord.x * (1.0f - texture_coord.x), 0.2, 0.8) * clamp (texture_coord.y * (1.0f - texture_coord.y), 0.2, 0.8);
//    fragment_color.w = 0.5 + 0.5 * cnoise(30*texture_coord);
    fragment_color.w = 0.5 + 0.5 * cnoise(300 * texture_coord);
//  fragment_color.w = 128 * (0.25 - clamp (texture_coord.x * (1.0f - texture_coord.x), 0.2, 0.8)) * (0.25 - clamp (texture_coord.y * (1.0f - texture_coord.y), 0.2, 0.8));

};

