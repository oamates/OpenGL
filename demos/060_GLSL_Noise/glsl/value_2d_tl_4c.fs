#version 330

uniform sampler2D value_texture;
in vec2 v_texCoord2D;

out vec4 FragmentColor;

//==============================================================================================================================================================
// Smooth interpolants between 0 (x < a) and 1 (x > b) values.
// Has zero derivative(s) at x = a and x = b up to the corresponding order (1 for hermite3, 2 for hermite5 and 3 for hermite7)
//==============================================================================================================================================================

float hermite3(float a, float b, float x)
{
    x = clamp((x - a) / (b - a), 0.0, 1.0);                                     // linearly transform the argument to get y in 0..1 range
    return x * x * (3.0 - 2.0 * x);                                             // interpolation polynomial of degree 3
}

vec2 hermite3(float a, float b, vec2 x)
{
    x = clamp((x - a) / (b - a), 0.0, 1.0);
    return x * x * (3.0 - 2.0 * x);
}

vec3 hermite3(float a, float b, vec3 x)
{
    x = clamp((x - a) / (b - a), 0.0, 1.0);
    return x * x * (3.0 - 2.0 * x);
}

float hermite5(float a, float b, float x)
{
    x = clamp((x - a) / (b - a), 0.0, 1.0);
    return x * x * x * (10.0 + x * (6.0 * x - 15.0));                           // interpolation polynomial of order 5
}

vec2 hermite5(float a, float b, vec2 x)
{
    x = clamp((x - a) / (b - a), 0.0, 1.0);
    return x * x * x * (10.0 + x * (6.0 * x - 15.0));
}

vec3 hermite5(float a, float b, vec3 x)
{
    x = clamp((x - a) / (b - a), 0.0, 1.0);
    return x * x * x * (10.0 + x * (6.0 * x - 15.0));
}

float hermite7(float a, float b, float x)
{
    x = clamp((x - a) / (b - a), 0.0, 1.0);
    float sqr = x * x;
    return sqr * sqr * (35.0 - 84.0 * x + sqr * (70.0 - 20.0 * x));             // interpolation polynomial of order 7
}

vec2 hermite7(float a, float b, vec2 x)
{
    x = clamp((x - a) / (b - a), 0.0, 1.0);
    vec2 sqr = x * x;
    return sqr * sqr * (35.0 - 84.0 * x + sqr * (70.0 - 20.0 * x));
}

vec3 hermite7(float a, float b, vec3 x)
{
    x = clamp((x - a) / (b - a), 0.0, 1.0);
    vec3 sqr = x * x;
    return sqr * sqr * (35.0 - 84.0 * x + sqr * (70.0 - 20.0 * x));
}

//==============================================================================================================================================================
// Special case of hermite interpolants for a = 0, b = 1.
//==============================================================================================================================================================

float hermite3(float x)
{
    return x * x * (3.0 - 2.0 * x);
}

float hermite5(float x)
{
    return x * x * x * (10.0 + x * (6.0 * x - 15.0));
}

float hermite7(float x)
{
    float sqr = x * x;
    return sqr * sqr * (35.0 - 84.0 * x + sqr * (70.0 - 20.0 * x));
}

vec2 hermite3(vec2 x)
{
    return x * x * (3.0 - 2.0 * x);
}

vec2 hermite5(vec2 x)
{
    return x * x * x * (10.0 + x * (6.0 * x - 15.0));
}

vec2 hermite7(vec2 x)
{
    vec2 sqr = x * x;
    return sqr * sqr * (35.0 - 84.0 * x + sqr * (70.0 - 20.0 * x));
}

vec3 hermite3(vec3 x)
{
    return x * x * (3.0 - 2.0 * x);
}

vec3 hermite5(vec3 x)
{
    return x * x * x * (10.0 + x * (6.0 * x - 15.0));
}

vec3 hermite7(vec3 x)
{
    vec3 sqr = x * x;
    return sqr * sqr * (35.0 - 84.0 * x + sqr * (70.0 - 20.0 * x));
}


//==============================================================================================================================================================
// 2D Value noise function : adding HALF_TEXEL is important 
//==============================================================================================================================================================

#define TEXEL_SIZE 1.0 / 256.0
#define HALF_TEXEL 1.0 / 512.0

vec4 vnoise(in vec2 x)
{
    vec2 p = floor(x);
    vec2 f = x - p;
//    vec2 uv = p + hermite3(f);
    vec2 uv = p + hermite5(f);
//    vec2 uv = p + hermite7(f);
    return texture(value_texture, TEXEL_SIZE * uv + HALF_TEXEL);
}


vec4 fract_vnoise(vec2 uv)
{
    const mat2 r = mat2(vec2( 0.6, -0.8), 
                        vec2( 0.8,  0.6));
    const int level = 6;
    float frequency = 1.0;
    float amplitude = 0.5;
    vec4 q = amplitude * vnoise(v_texCoord2D);

    for(int i = 0; i < level; ++i)
    {
        frequency += frequency;
        amplitude *= 0.5;    
        uv = r * uv;
        q += amplitude * vnoise(frequency * uv);
    }
    return q;
}

void main()
{
    vec4 q = fract_vnoise(v_texCoord2D);

    vec3 rgb0 = vec3(q.x * q.y, q.x, q.x * q.y * q.z);
    vec3 rgb1 = vec3(q.y * q.z, q.y, q.y * q.z * q.w);
    vec3 rgb2 = vec3(q.z * q.w, q.z, q.z * q.w * q.x);
    vec3 rgb3 = vec3(q.w * q.x, q.w, q.w * q.x * q.y);
    vec3 rgb4 = vec3(q.x * q.x, q.x, q.x * q.x * q.x);

    vec3 rgb;

    float x = q.x;

    if (x > 0.75)
        rgb = mix(rgb1, rgb0, hermite5(0.75,  1.0, x));
    else if (x > 0.50)
        rgb = mix(rgb2, rgb1, hermite5(0.50, 0.75, x));
    else if (x > 0.25)
        rgb = mix(rgb3, rgb2, hermite5(0.25, 0.50, x));
    else
        rgb = mix(rgb4, rgb3, hermite5( 0.0, 0.25, x));

    FragmentColor = vec4(rgb, 1.0);

}
