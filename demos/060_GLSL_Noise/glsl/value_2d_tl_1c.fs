#version 330

uniform sampler2D value_texture;
in vec2 v_texCoord2D;

out vec4 FragmentColor;

//==============================================================================================================================================================
// Smooth interpolants between 0 (x < a) and 1 (x > b) values.
// Has zero derivative(s) at x = a and x = b up to the corresponding order (1 for smoothstep3, 2 for smoothstep 5 and 3 for smoothstep 7)
//==============================================================================================================================================================

float hermite3(float a, float b, float x)
{
    // linearly transform the argument to get y in 0..1 range
    float y = clamp((x - a) / (b - a), 0.0, 1.0); 
    // Hermite polynomial of order 3
    return y * y * (3.0 - 2.0 * y);
}

vec2 hermite3(float a, float b, vec2 x)
{
    // linearly transform the argument to get y in 0..1 range
    vec2 y = clamp((x - a) / (b - a), 0.0, 1.0); 
    // Hermite polynomial of order 3
    return y * y * (3.0 - 2.0 * y);
}

float hermite5(float a, float b, float x)
{
    // linearly transform the argument to get y in 0..1 range
    float y = clamp((x - a) / (b - a), 0.0, 1.0);
    // Hermite polynomial of order 5
    return y * y * y * (10.0 + y * (15.0 - 6.0 * y));
}

vec2 hermite5(float a, float b, vec2 x)
{
    // linearly transform the argument to get y in 0..1 range
    vec2 y = clamp((x - a) / (b - a), 0.0, 1.0);
    // Hermite polynomial of order 5
    return y * y * y * (10.0 + y * (6.0 * y - 15.0));
}

float hermite7(float a, float b, float x)
{
    // Scale, and clamp x to 0..1 range
    float y = clamp((x - a) / (b - a), 0.0, 1.0);
    float sqr = y * y;
    // Evaluate polynomial
    return sqr * sqr * (35.0 - 84.0 * x + sqr * (70.0 - 20.0 * x));
}

//==============================================================================================================================================================
// Special case of hermite interpolants for a = 0, b = 1.
//==============================================================================================================================================================

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


//==============================================================================================================================================================
// 2D Value noise function : adding HALF_TEXEL is important 
//==============================================================================================================================================================

#define TEXEL_SIZE 1.0 / 256.0
#define HALF_TEXEL 1.0 / 512.0

float vnoise(in vec2 x)
{
    vec2 p = floor(x);
    vec2 f = x - p;
    vec2 uv = p + hermite3(f);
//    vec2 uv = p + hermite5(f);
//    vec2 uv = p + hermite7(f);
    return texture(value_texture, TEXEL_SIZE * uv + HALF_TEXEL).x;
}


void main()
{
    float n = vnoise(v_texCoord2D);
    FragmentColor = vec4(vec3(n, n, n), 1.0);
}
