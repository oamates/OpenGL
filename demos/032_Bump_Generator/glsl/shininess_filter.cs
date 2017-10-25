#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

//==============================================================================================================================================================
// Every workgroup will work on 8 x 8 pixel area
//==============================================================================================================================================================

uniform sampler2D diffuse_tex;
layout (r32f) uniform image2D shininess_image;

uniform vec2 texel_size;

//==============================================================================================================================================================
// auxiliary rgb --> hsv and hsv --> rgb routines
//==============================================================================================================================================================
vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}
 

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

//==============================================================================================================================================================
// shader entry point
//==============================================================================================================================================================
void main(void)
{
    ivec2 P = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv = texel_size * (vec2(P) + 0.5);

    vec3 c = texture(diffuse_tex, uv).rgb;
    vec3 hsv = rgb2hsv(c);
    hsv.y = 0.0f;
    vec3 rgb = hsv2rgb(hsv);

    float q = clamp(0.17 * dot(rgb, rgb), 0.0, 1.0);
    q = pow(q, 0.7);

    imageStore(shininess_image, P, vec4(q, 0.0, 0.0, 0.0));
}