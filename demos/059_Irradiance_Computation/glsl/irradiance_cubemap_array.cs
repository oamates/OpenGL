#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

const int MAX_LEVEL = 10;

//==============================================================================================================================================================
// input :: cubemap radiance texture array
// output :: buffer of the spherical harmonic coefficients
//==============================================================================================================================================================
uniform int level;
uniform int points;

//==============================================================================================================================================================
// Real spherical harmonics for levels 0,1,2,3
//==============================================================================================================================================================
//
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
//


float m = 0.0f;
vec3 m_xyz = value * n;
vec3 m_xy_yz_zx = value * n * n.yzx;
vec3 m_xx_yy_zz = value * n * n;


//==============================================================================================================================================================
// Function to calculate irradiance spherical harmonic coefficients from cubemap texture Real spherical harmonics for levels 0,1,2,3
//==============================================================================================================================================================
void accumulate (vec3 n, float dA)
{
    float v;


    /* CUBEMAP_NEGATIVE_Z :: { x, y, z} */

    vec3 xyz = n;
    vec3 xy_yz_zx = n * n.yzx;
    vec3 xx_yy_zz = n * n;

    v = texture(cubemapTex, n) * dA;

    vec3 v_xyz      = v * xyz;
    vec3 v_xy_yz_zx = v * xy_yz_zx;
    vec3 v_xx_yy_zz = v * xx_yy_zz;

    m          += v;
    m_xyz      += v_xyz;
    m_xy_yz_zx += v_xy_yz_zx;
    m_xx_yy_zz += v_xx_yy_zz;

    /* CUBEMAP_POSITIVE_Z :: {-x, y,-z} */

    vec3 xyz = n;
    vec3 xy_yz_zx = n * n.yzx;
    vec3 xx_yy_zz = n * n;

    vec3 v_xyz      = v * xyz;
    vec3 v_xy_yz_zx = v * xy_yz_zx;
    vec3 v_xx_yy_zz = v * xx_yy_zz;

    m          += v;
    m_xyz      += v_xyz;
    m_xy_yz_zx += v_xy_yz_zx;
    m_xx_yy_zz += v_xx_yy_zz;

    /* CUBEMAP_NEGATIVE_Y :: { x,-z, y} */
    vec3 xyz = n;
    vec3 xy_yz_zx = n * n.yzx;
    vec3 xx_yy_zz = n * n;

    vec3 v_xyz      = v * xyz;
    vec3 v_xy_yz_zx = v * xy_yz_zx;
    vec3 v_xx_yy_zz = v * xx_yy_zz;

    m          += v;
    m_xyz      += v_xyz;
    m_xy_yz_zx += v_xy_yz_zx;
    m_xx_yy_zz += v_xx_yy_zz;

    /* CUBEMAP_POSITIVE_Y :: { x, z,-y} */
    vec3 xyz = n;
    vec3 xy_yz_zx = n * n.yzx;
    vec3 xx_yy_zz = n * n;

    vec3 v_xyz      = v * xyz;
    vec3 v_xy_yz_zx = v * xy_yz_zx;
    vec3 v_xx_yy_zz = v * xx_yy_zz;

    m          += v;
    m_xyz      += v_xyz;
    m_xy_yz_zx += v_xy_yz_zx;
    m_xx_yy_zz += v_xx_yy_zz;

    /* CUBEMAP_NEGATIVE_X :: {-z, y, x} */
    vec3 xyz = n;
    vec3 xy_yz_zx = n * n.yzx;
    vec3 xx_yy_zz = n * n;

    vec3 v_xyz      = v * xyz;
    vec3 v_xy_yz_zx = v * xy_yz_zx;
    vec3 v_xx_yy_zz = v * xx_yy_zz;

    m          += v;
    m_xyz      += v_xyz;
    m_xy_yz_zx += v_xy_yz_zx;
    m_xx_yy_zz += v_xx_yy_zz;

    /* CUBEMAP_POSITIVE_X :: { z, y,-x} */

    vec3 xyz = n;
    vec3 xy_yz_zx = n * n.yzx;
    vec3 xx_yy_zz = n * n;

    vec3 v_xyz      = v * xyz;
    vec3 v_xy_yz_zx = v * xy_yz_zx;
    vec3 v_xx_yy_zz = v * xx_yy_zz;

    m          += v;
    m_xyz      += v_xyz;
    m_xy_yz_zx += v_xy_yz_zx;
    m_xx_yy_zz += v_xx_yy_zz;
}

//
samplerCubeArray

    mat3(vec3( 0.0f,  0.0f, -1.0f),       (-n.z, -n.y, -n.x)
         vec3( 0.0f, -1.0f,  0.0f),
         vec3(-1.0f,  0.0f,  0.0f)),

    mat3(vec3( 0.0f,  0.0f,  1.0f),       ( n.z, -n.y,  n.x)
         vec3( 0.0f, -1.0f,  0.0f),
         vec3( 1.0f,  0.0f,  0.0f)),

    mat3(vec3( 1.0f,  0.0f,  0.0f),       ( n.x, -n.z,  n.y)
         vec3( 0.0f,  0.0f, -1.0f),
         vec3( 0.0f,  1.0f,  0.0f)),
    mat3(vec3( 1.0f,  0.0f,  0.0f),       ( n.x,  n.z, -n.y)
         vec3( 0.0f,  0.0f,  1.0f),
         vec3( 0.0f, -1.0f,  0.0f)),
    mat3(vec3( 1.0f,  0.0f,  0.0f),       ( n.x, -n.y, -n.z)
         vec3( 0.0f, -1.0f,  0.0f),
         vec3( 0.0f,  0.0f, -1.0f)),
    mat3(vec3(-1.0f,  0.0f,  0.0f),       (-n.x, -n.y,  n.z)
         vec3( 0.0f, -1.0f,  0.0f),
         vec3( 0.0f,  0.0f,  1.0f))

    vec3(-1.0f, -1.0f, -1.0f),
    vec3( 1.0f, -1.0f, -1.0f),
    vec3(-1.0f,  1.0f, -1.0f),
    vec3( 1.0f,  1.0f, -1.0f),
    vec3(-1.0f, -1.0f,  1.0f),
    vec3( 1.0f, -1.0f,  1.0f),
    vec3(-1.0f,  1.0f,  1.0f),
    vec3( 1.0f,  1.0f,  1.0f)
















void main()
{

    vec3 r = normalize(ray);
    float l = length(r.xy);
    vec2 uv = 0.5 + inv_4pi_2pi * vec2(atan(r.x, r.y), asin(r.z));
    FragmentColor = texture(equirectangular_map, uv);


// gvec4 texture(gsamplerCubeArray sampler, vec4 P, [float bias]);
}
for ()
