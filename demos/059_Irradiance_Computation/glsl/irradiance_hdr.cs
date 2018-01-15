
#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

in uvec3 gl_GlobalInvocationID;

//==============================================================================================================================================================
// input :: HDR rectangular texture and its size
//==============================================================================================================================================================
uniform sampler2D hdr_tex;
uniform vec2 texel_size;

uniform int level;
uniform int points;

//==============================================================================================================================================================
// Real spherical harmonics for levels 0,1,2,3
//==============================================================================================================================================================
// Y_0_0 = 1 / (2 * sqrt(pi)) = 0.28209479177f
//
// Y_1_m1 = sqrt(3) / (2 * sqrt(pi)) * y = 0.4886025119f * y
// Y_1_0  = sqrt(3) / (2 * sqrt(pi)) * z = 0.4886025119f * z
// Y_1_p1 = sqrt(3) / (2 * sqrt(pi)) * x = 0.4886025119f * x
//
// Y_2_m2 = sqrt(15) / (2 * sqrt(pi)) * xy = 1.09254843059f * xy
// Y_2_m1 = sqrt(15) / (2 * sqrt(pi)) * yz = 1.09254843059f * yz
// Y_2_0  = sqrt(5) / (4 * sqrt(pi)) * (-xx - yy + 2zz) = 0.31539156525f * (-xx - yy + 2zz)
// Y_2_p1 = sqrt(15) / (2 * sqrt(pi)) * zx = 1.09254843059f * zx
// Y_2_p2 = sqrt(15) / (4 * sqrt(pi)) * (xx - yy) = 0.54627421529f * (xx - yy)
//
// Y_3_m3 = sqrt(35) / (4 * sqrt(2 * pi)) * (3xx - yy) * y = 0.59004358992f * (3xx - yy) * y
// Y_3_m2 = sqrt(105) / (2 * sqrt(pi)) * xyz = 2.89061144264f * xyz
// Y_3_m1 = sqrt(21) / (4 * sqrt(2 * pi)) * (4zz - xx - yy) * y = 0.45704579946f * (4zz - xx - yy) * y
// Y_3_0  = sqrt(7) / (4 * sqrt(pi)) * (2zz - 3xx - 3yy) * z = 0.37317633259f * (2zz - 3xx - 3yy) * z
// Y_3_p1 = sqrt(21) / (4 * sqrt(2 * pi)) * (4zz - xx - yy) * x = 0.45704579946f * (4zz - xx - yy) * x
// Y_3_p2 = sqrt(105) / (4 * sqrt(pi)) * (xx - yy) * z = 1.44530572132f * (xx - yy) * z
// Y_3_p3 = sqrt(35) / (4 * sqrt(2 * pi)) * (xx - 3yy) * x = 0.59004358992f * (xx - 3yy) * x




//==============================================================================================================================================================
// Function to calculate irradiance (level 0, 1, 2) spherical harmonic coefficients from equirectangulat HDR texture
//==============================================================================================================================================================

const float pi = 3.14159265359f;
const float two_pi = 6.28318530718f;

void main()
{
    vec2 uv = texel_size * (vec2(gl_GlobalInvocationID.xy) + vec2(0.5));

    vec3 rgb = texture(hdr_tex, uv);

    vec3 rgb_x = rgb * x;
    vec3 rgb_y = rgb * y;
    vec3 rgb_z = rgb * z;

    vec3 rgb_xy = rgb_x * y;
    vec3 rgb_yz = rgb_y * z;
    vec3 rgb_zx = rgb_z * x;

    vec3 rgb_xx = rgb_x * x;
    vec3 rgb_yy = rgb_y * y;
    vec3 rgb_zz = rgb_z * z;


float m;
vec3 m_xyz = value * n;
vec3 m_xy_yz_zx = value * n * n.yzx;
vec3 m_xx_yy_zz = value * n * n;

// gvec4 texture(gsamplerCubeArray sampler, vec4 P, [float bias]);
}
for ()
