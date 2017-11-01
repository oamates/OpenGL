#version 430 core

layout (local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

int iX = int(gl_GlobalInvocationID.x);
int iY = int(gl_GlobalInvocationID.y);

layout (local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

#define m 12
#define n (1 << m)

int iX = int(gl_GlobalInvocationID.x);
int iY = int(gl_GlobalInvocationID.y);

//========================================================================================================================================================================================================================
// load buffer :: the load data is a complex (packed as vec2) N x N matrix
// No bit reversing step is performed, therefore the first butterfly step should be different from the
// following and essentially perform both bit reversing and butterfly.
// It make sence to separate it in any case as for the first butterfly step roots of unity 
// are so simple that no multiplication is required at all
//========================================================================================================================================================================================================================
layout (rg32f, binding = 0) uniform imageBuffer input_buffer;
layout (rg32f, binding = 1) uniform imageBuffer inout_buffer;

shared vec2 Z[n];


//==============================================================================================================================================================
// Every workgroup will work on 8 x 8 pixel area
//==============================================================================================================================================================
uniform sampler2D input_tex;
layout (r32f) uniform image2D output_image;

uniform vec2 texel_size;

//==============================================================================================================================================================
// auxiliary rgb --> hsv and hsv --> rgb routines
//==============================================================================================================================================================
vec2 load(int index)
    { return imageLoad(input_buffer, index).rg; }

void store(int index, vec2 value)
    { imageStore(inout_buffer, index, vec4(value, 0.0, 0.0)); }

//==============================================================================================================================================================
// shader entry point
//==============================================================================================================================================================
void main()
{
    ivec2 P = ivec2(gl_GlobalInvocationID.xy);
    float q = imageLoad();

    vec2 uv = texel_size * (vec2(P) + 0.5);

    vec3 c = texture(diffuse_tex, uv).rgb;
    vec3 hsv = rgb2hsv(c);
    hsv.y = 0.0f;
    vec3 rgb = hsv2rgb(hsv);

    float q = clamp(0.17 * dot(rgb, rgb), 0.0, 1.0);
    q = pow(q, 0.7);

    imageStore(shininess_image, P, vec4(q, 0.0, 0.0, 0.0));
}