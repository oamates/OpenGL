#include <glm/gtc/type_ptr.hpp>

#include "sse41.hpp"
#include "common.hpp"
#include "simdhelp.hpp"

namespace noise {

//=======================================================================================================================================================================================================================
// SSE41 implementation of value noise                                                                                                                                                                                   
//=======================================================================================================================================================================================================================
template<> float value_sse41<float>(const glm::vec2& P) 
{
    return 0.0f;
}

    template<> double value_sse41<double>(const glm::dvec2& arg) 
    {
        __m128d P = _mm_load_pd(glm::value_ptr(arg));
        __m128d Pid = _mm_floor_pd(P);                                                                  // Pid = | [x] : [y] | as doubles
        __m128i Pi = _mm_cvtpd_epi32(Pid);                                                              // Pi  = | [x] : [y] : 0 : 0 | as integers
        __m128d Pf = _mm_sub_pd(P, Pid);                                                                // Pf  = | {x} : {y} | as doubles
        __m128d Ps = smooth_step5(Pf);                                                                  // Ps  = | s_x : s_y |

        Pi = _mm_unpacklo_epi64(Pi, Pi);                                                                // Pi  = | [x] : [y] : [x] : [y] |
        Pi = _mm_mullo_epi32(Pi, _m128i_HASH_FACTOR_XYXY);                                              // Pi  = | [x] * F_X : [y] * F_Y : [x] * F_X : [y] * F_Y | lower 32-bit parts
        Pi = _mm_hadd_epi32(Pi, Pi);                                                                    // Pi  = | [x] * F_X + [y] * F_Y | in all 4 components
        __m128i hash_factor = _mm_set1_epi32(HASH_FACTOR);
        __m128i hash = _mm_add_epi32(Pi, _m128i_LINEAR_SHIFT_0_X_Y_XY);
        hash = _mm_xor_si128(hash, _mm_srli_epi32(hash, HASH_SHIFT));
        hash = _mm_mullo_epi32(hash, hash_factor);                                                      // hash = | hash(0, 0) : hash(1, 0) : hash(0, 1) : hash(1, 1) |

        __m128d val_00_10 = _mm_cvtepi32_pd(hash);
        __m128d val_01_11 = _mm_cvtepi32_pd(_mm_unpackhi_epi64(hash, hash));
        __m128d val_x01 = _mm_add_pd(val_00_10, _mm_mul_pd(_mm_sub_pd(val_01_11, val_00_10), _mm_unpackhi_pd(Ps, Ps)));
        __m128d val = _mm_add_sd(val_x01, _mm_mul_sd(_mm_sub_sd(_mm_unpackhi_pd(val_x01, val_x01), val_x01), Ps));
        return _mm_cvtsd_f64(val) * VALUE_2D_SSE41_NORMALIZATION_FACTOR;
    }

template<> float value_sse41(const glm::vec3& P)
{
    return 0.0f;
}

template<> double value_sse41(const glm::dvec3& P)
{
    return 0.0;
}

    template<> float value_sse41(const glm::vec4& arg)
    {
        __m128 P = _mm_load_ps(glm::value_ptr(arg));                                                // P   = |  x  :  y  :  z  :  w  |
        __m128 Pid = _mm_floor_ps(P);                                                               // Pid = | [x] : [y] : [z] : [w] | as doubles
        __m128i Pi = _mm_cvtps_epi32(Pid);                                                          // Pi  = | [x] : [y] : [z] : [w] | as integers

        __m128 Pf = _mm_sub_ps(P, Pid);                                                             // Pf  = | {x} : {y} : {z} : {w}  |

        __m128i Pm = _mm_mullo_epi32(Pi, _m128i_LINEAR_FACTOR_XYZW);                                // Pm = | [x] * F_X : [y] * F_Y : [z] * F_Z : [w] * F_W |
        Pm = _mm_hadd_epi32(Pm, Pm);
        Pm = _mm_hadd_epi32(Pm, Pm);                                                                
        __m128i Pm00 = _mm_add_epi32(Pm, _m128i_LINEAR_SHIFT_0_Y_X_YX);                             // | L(0,0,0,0) : L(0,1,0,0) : L(1,0,0,0) : L(1,1,0,0) |

        __m128i Fzzzz = _mm_shuffle_epi32(_m128i_LINEAR_FACTOR_XYZW, _MM_SHUFFLE(2,2,2,2));         // | F_Z : F_Z : F_Z : F_Z |
        __m128i Fwwww = _mm_shuffle_epi32(_m128i_LINEAR_FACTOR_XYZW, _MM_SHUFFLE(3,3,3,3));         // | F_W : F_W : F_W : F_W |

        __m128i Pm10 = _mm_add_epi32(Pm00, Fzzzz);                                                  // | L(0,0,1,0) : L(1,0,1,0) : L(0,1,1,0) : L(1,1,1,0) |
        __m128i Pm01 = _mm_add_epi32(Pm00, Fwwww);                                                  // | L(0,0,0,1) : L(1,0,0,1) : L(0,1,0,1) : L(1,1,0,1) |
        __m128i Pm11 = _mm_add_epi32(Pm10, Fwwww);                                                  // | L(0,0,1,1) : L(1,0,1,1) : L(0,1,1,1) : L(1,1,1,1) |

        __m128i hash_factor = _mm_set1_epi32(HASH_FACTOR);

        Pm00 = _mm_mullo_epi32(_mm_xor_si128(Pm00, _mm_srli_epi32(Pm00, HASH_SHIFT)), hash_factor); // | hash (0,0,0,0) : hash (1,0,0,0) : hash (0,1,0,0) : hash (1,1,0,0) |
        Pm10 = _mm_mullo_epi32(_mm_xor_si128(Pm10, _mm_srli_epi32(Pm10, HASH_SHIFT)), hash_factor); // | hash (0,0,1,0) : hash (1,0,1,0) : hash (0,1,1,0) : hash (1,1,1,0) |
        Pm01 = _mm_mullo_epi32(_mm_xor_si128(Pm01, _mm_srli_epi32(Pm01, HASH_SHIFT)), hash_factor); // | hash (0,0,0,1) : hash (1,0,0,1) : hash (0,1,0,1) : hash (1,1,0,1) |
        Pm11 = _mm_mullo_epi32(_mm_xor_si128(Pm11, _mm_srli_epi32(Pm11, HASH_SHIFT)), hash_factor); // | hash (0,0,1,1) : hash (1,0,1,1) : hash (0,1,1,1) : hash (1,1,1,1) |

        __m128 Ps = smooth_step5(Pf);                                                               // Ps   = | s_x : s_y : s_z : s_w |

        // interpolate with respect to w-component
        __m128 val_zw10 = _mm_cvtepi32_ps(Pm10);                                                    // | hash (0,0,1,0) : hash (1,0,1,0) : hash (0,1,1,0) : hash (1,1,1,0) | as floats
        __m128 val_zw11 = _mm_cvtepi32_ps(Pm11);                                                    // | hash (0,0,1,1) : hash (1,0,1,1) : hash (0,1,1,1) : hash (1,1,1,1) | as floats

        __m128 ws_ws_ws_ws = _mm_shuffle_ps(Ps, Ps, _MM_SHUFFLE(3,3,3,3));                          // ws_ws_ws_ws = | ws : ws : ws : ws |
        __m128 val_z1 = _mm_add_ps(val_zw10, _mm_mul_ps(_mm_sub_ps(val_zw11, val_zw10), ws_ws_ws_ws));

        __m128 val_zw00 = _mm_cvtepi32_ps(Pm00);                                                    // | hash (0,0,0,0) : hash (1,0,0,0) : hash (0,1,0,0) : hash (1,1,0,0) | as floats
        __m128 val_zw01 = _mm_cvtepi32_ps(Pm01);                                                    // | hash (0,0,0,1) : hash (1,0,0,1) : hash (0,1,0,1) : hash (1,1,0,1) | as floats
        __m128 val_z0 = _mm_add_ps(val_zw00, _mm_mul_ps(_mm_sub_ps(val_zw01, val_zw00), ws_ws_ws_ws));

        // interpolate with respect to z-component
        __m128 zs_zs_zs_zs = _mm_shuffle_ps(Ps, Ps, _MM_SHUFFLE(2,2,2,2));                          // zs_zs_zs_zs = | zs : zs : zs : zs |
        __m128 val = _mm_add_ps(val_z0, _mm_mul_ps(_mm_sub_ps(val_z1, val_z0), zs_zs_zs_zs));

        // interpolate with respect to y-component
        hash_factor = _mm_cmpeq_epi32(hash_factor, hash_factor);
        __m128 maskf = _mm_castsi128_ps(_mm_slli_epi32(_mm_srli_epi32(hash_factor, 25), 23));       // reuse integer register to avoid extra PXOR instruction
        __m128 xs_xs_ys_ys = _mm_unpackhi_ps(Ps, Ps);                                               // xs_xs_ys_ys = | xs : xs : ys : ys |
        __m128 _1mxs_1mxs_1mys_1mys = _mm_sub_ps(maskf, Ps);                                        // _1mxs_1mxs_1mys_1mys = | 1 - xs : 1 - xs : 1 - ys : 1 - ys |
        __m128 _1mys_ys_1mys_ys = _mm_unpackhi_ps(_1mxs_1mxs_1mys_1mys, xs_xs_ys_ys);               // _1mxs_1mxs_ys_ys = | 1 - ys : ys : 1 - ys : ys |
        __m128 _1mxs_xs_1mxs_xs = _mm_unpacklo_ps(_1mxs_1mxs_1mys_1mys, xs_xs_ys_ys);               // _1mxs_xs_1mxs_xs = | 1 - xs : xs : 1 - xs : xs |
        __m128 _1mxs_1mxs_xs_xs = _mm_unpacklo_ps(_1mxs_xs_1mxs_xs, _1mxs_xs_1mxs_xs);              // _1mxs_1mxs_xs_xs = | 1 - xs : 1 - xs : xs : xs |
        val = _mm_mul_ps(val, _1mys_ys_1mys_ys);
        val = _mm_mul_ps(val, _1mxs_1mxs_xs_xs);
        val = _mm_hadd_ps(val, val);
        val = _mm_hadd_ps(val, val);

        return _mm_cvtss_f32(val) * VALUE_4D_SSE41_NORMALIZATION_FACTOR_F;
    }

    template<> double value_sse41(const glm::dvec4& arg)
    {
        __m128d Pxy = _mm_load_pd(&arg.x);                                                          // Pxy    = |  x  :  y  |
        __m128d Pzw = _mm_load_pd(&arg.z);                                                          // Pzw    = |  z  :  w  |
        __m128d Pxyid = _mm_floor_pd(Pxy);                                                          // Pxyid  = | [x] : [y] | as doubles
        __m128d Pzwid = _mm_floor_pd(Pzw);                                                          // Pzwid  = | [z] : [w] | as doubles
        __m128i Pxyi = _mm_cvtpd_epi32(Pxyid);                                                      // Pxyi   = | [x] : [y] : 0 : 0 | as integers
        __m128i Pzwi = _mm_cvtpd_epi32(Pzwid);                                                      // Pzwi   = | [z] : [w] : 0 : 0 | as integers

        __m128d Pxyf = _mm_sub_pd(Pxy, Pxyid);                                                      // Pxy    = | {x} : {y} |
        __m128d Pzwf = _mm_sub_pd(Pzw, Pzwid);                                                      // Pzw    = | {z} : {w} |

        __m128i Pxyzwi = _mm_unpacklo_epi64(Pxyi, Pzwi);                                            // Pxyzwi = | [x] : [y] : [z] : [w] |
        __m128i Pm = _mm_mullo_epi32(Pxyzwi, _m128i_LINEAR_FACTOR_XYZW);                            // Pm = | [x] * F_X : [y] * F_Y : [z] * F_Z : [w] * F_W |
        Pm = _mm_hadd_epi32(Pm, Pm);
        Pm = _mm_hadd_epi32(Pm, Pm);                                                                
        __m128i Pm00 = _mm_add_epi32(Pm, _m128i_LINEAR_SHIFT_0_X_Y_XY);                             // | L(0,0,0,0) : L(1,0,0,0) : L(0,1,0,0) : L(1,1,0,0) |

        __m128i Fzzzz = _mm_shuffle_epi32(_m128i_LINEAR_FACTOR_XYZW, _MM_SHUFFLE(2,2,2,2));         // | F_Z : F_Z : F_Z : F_Z |
        __m128i Fwwww = _mm_shuffle_epi32(_m128i_LINEAR_FACTOR_XYZW, _MM_SHUFFLE(3,3,3,3));         // | F_W : F_W : F_W : F_W |

        __m128i Pm10 = _mm_add_epi32(Pm00, Fzzzz);                                                  // | L(0,0,1,0) : L(1,0,1,0) : L(0,1,1,0) : L(1,1,1,0) |
        __m128i Pm01 = _mm_add_epi32(Pm00, Fwwww);                                                  // | L(0,0,0,1) : L(1,0,0,1) : L(0,1,0,1) : L(1,1,0,1) |
        __m128i Pm11 = _mm_add_epi32(Pm10, Fwwww);                                                  // | L(0,0,1,1) : L(1,0,1,1) : L(0,1,1,1) : L(1,1,1,1) |

        __m128i hash_factor = _mm_set1_epi32(HASH_FACTOR);

        Pm00 = _mm_mullo_epi32(_mm_xor_si128(Pm00, _mm_srli_epi32(Pm00, HASH_SHIFT)), hash_factor); // | hash (0,0,0,0) : hash (1,0,0,0) : hash (0,1,0,0) : hash (1,1,0,0) |
        Pm10 = _mm_mullo_epi32(_mm_xor_si128(Pm10, _mm_srli_epi32(Pm10, HASH_SHIFT)), hash_factor); // | hash (0,0,1,0) : hash (1,0,1,0) : hash (0,1,1,0) : hash (1,1,1,0) |
        Pm01 = _mm_mullo_epi32(_mm_xor_si128(Pm01, _mm_srli_epi32(Pm01, HASH_SHIFT)), hash_factor); // | hash (0,0,0,1) : hash (1,0,0,1) : hash (0,1,0,1) : hash (1,1,0,1) |
        Pm11 = _mm_mullo_epi32(_mm_xor_si128(Pm11, _mm_srli_epi32(Pm11, HASH_SHIFT)), hash_factor); // | hash (0,0,1,1) : hash (1,0,1,1) : hash (0,1,1,1) : hash (1,1,1,1) |

        __m128d Pxys = smooth_step5(Pxyf);                                                          // Pxys   = | s_x : s_y |
        __m128d Pzws = smooth_step5(Pzwf);                                                          // Pzws   = | s_z : s_w |

        // interpolate with respect to w-component for z = 1 :: Pm11 and Pm10 values
        __m128d val_zw10_0 = _mm_cvtepi32_pd(Pm10);                                                 // | hash (0,0,1,0) : hash (1,0,1,0) | as doubles
        __m128d val_zw10_1 = _mm_cvtepi32_pd(_mm_srli_si128(Pm10, 8));                              // | hash (0,1,1,0) : hash (1,1,1,0) | as doubles
        __m128d val_zw11_0 = _mm_cvtepi32_pd(Pm11);                                                 // | hash (0,0,1,1) : hash (1,0,1,1) | as doubles
        __m128d val_zw11_1 = _mm_cvtepi32_pd(_mm_srli_si128(Pm11, 8));                              // | hash (0,1,1,1) : hash (1,1,1,1) | as doubles

        __m128d ws_ws = _mm_unpackhi_pd(Pzws, Pzws);                                                // ws_ws = | ws : ws |
        __m128d val_z1_0 = _mm_add_pd(val_zw10_0, _mm_mul_pd(_mm_sub_pd(val_zw11_0, val_zw10_0), ws_ws));
        __m128d val_z1_1 = _mm_add_pd(val_zw10_1, _mm_mul_pd(_mm_sub_pd(val_zw11_1, val_zw10_1), ws_ws));


        // interpolate with respect to w-component for z = 0 :: Pm01 and Pm00 values
        __m128d val_zw00_0 = _mm_cvtepi32_pd(Pm00);                                                 // | hash (0,0,0,0) : hash (1,0,0,0) | as doubles
        __m128d val_zw00_1 = _mm_cvtepi32_pd(_mm_srli_si128(Pm10, 8));                              // | hash (0,1,0,0) : hash (1,1,0,0) | as doubles
        __m128d val_zw01_0 = _mm_cvtepi32_pd(Pm01);                                                 // | hash (0,0,0,1) : hash (1,0,0,1) | as doubles
        __m128d val_zw01_1 = _mm_cvtepi32_pd(_mm_srli_si128(Pm11, 8));                              // | hash (0,1,0,1) : hash (1,1,0,1) | as doubles

        __m128d val_z0_0 = _mm_add_pd(val_zw00_0, _mm_mul_pd(_mm_sub_pd(val_zw01_0, val_zw00_0), ws_ws));
        __m128d val_z0_1 = _mm_add_pd(val_zw00_1, _mm_mul_pd(_mm_sub_pd(val_zw01_1, val_zw00_1), ws_ws));

        // interpolate with respect to z-component
        __m128d zs_zs = _mm_unpacklo_pd(Pzws, Pzws);                                                // zs_zs = | zs : zs |
        __m128d val_0 = _mm_add_pd(val_z0_0, _mm_mul_pd(_mm_sub_pd(val_z1_0, val_z0_0), zs_zs));
        __m128d val_1 = _mm_add_pd(val_z0_1, _mm_mul_pd(_mm_sub_pd(val_z1_1, val_z0_1), zs_zs));

        // interpolate with respect to y-component
        __m128d ys_ys = _mm_unpackhi_pd(Pxys, Pxys);                                                // zs_zs = | zs : zs |
        __m128d val = _mm_add_pd(val_0, _mm_mul_pd(_mm_sub_pd(val_1, val_0), ys_ys));
        val = _mm_add_sd(val, _mm_mul_sd(_mm_sub_sd(_mm_unpackhi_pd(val, val), val), Pxys));

        return val[0] * VALUE_4D_SSE41_NORMALIZATION_FACTOR;
    }
                                                                                                                                                                                                                         
//=======================================================================================================================================================================================================================
// SSE41 implementation of simplex noise                                                                                                                                                                                 
//=======================================================================================================================================================================================================================
template<> float simplex_sse41<float>(const glm::vec2& P) 
{
    return 0.0f;
}

    template<> double simplex_sse41<double>(const glm::dvec2& arg) 
    {
        __m128d __m128d_F2_F2 = _mm_unpacklo_pd(__m128d_F2_HALF, __m128d_F2_HALF);
        __m128d __m128d_G2_G2 = _mm_unpacklo_pd(__m128d_G2_G2M1, __m128d_G2_G2M1);

        __m128d P = _mm_loadu_pd(glm::value_ptr(arg));
        __m128d t = _mm_dp_pd(P, __m128d_F2_F2, 0x33);                                              // t   = | (P.x + P.y) * F2 : (P.x + P.y) * F2 |
        __m128d Pid = _mm_floor_pd(_mm_add_pd(P, t));                                               // Pi  = | [P.x + (P.x + P.y) * F2] : [P.y + (P.x + P.y) * F2] |
        __m128i Pi = _mm_cvtpd_epi32(Pid);                                                          // Pi  = | [P.x + (P.x + P.y) * F2] : [P.y + (P.x + P.y) * F2] : 0 : 0 | as integers
        t = _mm_dp_pd(Pid, __m128d_G2_G2, 0x33);
        __m128d P0 = _mm_sub_pd(Pid, t);

        __m128d Pf00 = _mm_sub_pd(P, P0);
        __m128d Pf10 = _mm_add_pd(Pf00, _mm_shuffle_pd(__m128d_G2_G2M1, __m128d_G2_G2M1, _MM_SHUFFLE2(0, 1)));
        __m128d Pf01 = _mm_add_pd(Pf00, __m128d_G2_G2M1);
        __m128d Pf11 = _mm_add_pd(Pf10, __m128d_G2_G2M1);

        Pi = _mm_unpacklo_epi64(Pi, Pi);                                                            // Pi = | [x] : [y] : [x] : [y] |
        Pi = _mm_mullo_epi32(Pi, _m128i_LINEAR_FACTOR_XYZW);                                          // Pi = | [x] * F_X : [y] * F_Y : [x] * F_Z : [y] * F_W |
        __m128i PiX0 = _mm_hadd_epi32(Pi, _mm_add_epi32(Pi, _m128i_SHIFT_0Y0W));                    // PiX0 = | [x] * F_X + [y] * F_Y : [x] * F_Z + [y] * F_W : [x] * F_X + ([y] + 1) * F_Y : [x] * F_Z + ([y] + 1) * F_W |
        __m128i PiX1 = _mm_add_epi32(PiX0, _m128i_SHIFT_XZXZ);                                      // PiX1 = | ([x] + 1) * F_X + [y] * F_Y : ([x] + 1) * F_Z + [y] * F_W : ([x] + 1) * F_X + ([y] + 1) * F_Y : ([x] + 1) * F_Z + ([y] + 1) * F_W |

        PiX0 = _mm_or_si128(PiX0, _mm_srli_epi32(PiX0, HASH_SHIFT));                                // PiX0 ^= (PiX0 >> HASH_SHIFT)
        PiX0 = _mm_mullo_epi32(PiX0, _m128i_HASH_FACTOR);                                           // PiX0 *= HASH_FACTOR :: | hash1(0, 0) : hash2(0, 0) : hash1(0, 1) : hash2(0, 1) |

        PiX1 = _mm_or_si128(PiX1, _mm_srli_epi32(PiX1, HASH_SHIFT));                                // PiX1 ^= (PiX1 >> HASH_SHIFT)
        PiX1 = _mm_mullo_epi32(PiX1, _m128i_HASH_FACTOR);                                           // PiX1 *= HASH_FACTOR :: | hash1(1, 0) : hash2(1, 1) : hash1(1, 1) : hash2(1, 1) |

        __m128d grad00 = _mm_cvtepi32_pd(PiX0);
        __m128d grad01 = _mm_cvtepi32_pd(_mm_unpackhi_epi64(PiX0, PiX0));
        __m128d grad10 = _mm_cvtepi32_pd(PiX1);
        __m128d grad11 = _mm_cvtepi32_pd(_mm_unpackhi_epi64(PiX1, PiX1));

        __m128d dot00_01 = _mm_unpacklo_pd(_mm_dp_pd(grad00, Pf00, 0x31), _mm_dp_pd(grad01, Pf01, 0x31));
        __m128d dot10_11 = _mm_unpacklo_pd(_mm_dp_pd(grad10, Pf10, 0x31), _mm_dp_pd(grad11, Pf11, 0x31));

        __m128d len00_01 = _mm_unpacklo_pd(_mm_dp_pd(Pf00, Pf00, 0x31), _mm_dp_pd(Pf01, Pf01, 0x31));
        __m128d len10_11 = _mm_unpacklo_pd(_mm_dp_pd(Pf10, Pf10, 0x31), _mm_dp_pd(Pf11, Pf11, 0x31));

        __m128d zero = _mm_setzero_pd();
        __m128d half = _mm_unpackhi_pd(__m128d_F2_HALF, __m128d_F2_HALF);;

        len00_01 = _mm_max_pd(_mm_sub_pd(half, len00_01), zero);
        len10_11 = _mm_max_pd(_mm_sub_pd(half, len10_11), zero);

        len00_01 = _mm_mul_pd(len00_01, len00_01);
        len10_11 = _mm_mul_pd(len10_11, len10_11);
        len00_01 = _mm_mul_pd(len00_01, len00_01);
        len10_11 = _mm_mul_pd(len10_11, len10_11);

        __m128d dotY0 = _mm_dp_pd(len00_01, dot00_01, 0x31);
        __m128d dotY1 = _mm_dp_pd(len10_11, dot10_11, 0x31);

        dotY0 = _mm_add_sd(dotY0, dotY1);
        return _mm_cvtsd_f64(dotY0) * SIMPLEX_2D_SSE41_NORMALIZATION_FACTOR;      
    }

template<> float simplex_sse41(const glm::vec3& P)
{
    return 0.0f;
}

template<> double simplex_sse41(const glm::dvec3& P)
{
    return 0.0;
}

template<> float simplex_sse41(const glm::vec4& P)
{
    return 0.0f;
}

template<> double simplex_sse41(const glm::dvec4& P)
{
    return 0.0;
}
                                                                                                                                                                                                                         
//=======================================================================================================================================================================================================================
// SSE41 implementation of gradient noise                                                                                                                                                                                
//=======================================================================================================================================================================================================================
template<> float gradient_sse41<float>(const glm::vec2& P) 
{
    return 0.0f;
}

    template<> double gradient_sse41<double>(const glm::dvec2& arg) 
    {        
        __m128d P = _mm_load_pd(glm::value_ptr(arg));
        __m128d Pid = _mm_floor_pd(P);                                                              // Pid  = | [x] : [y] | as doubles
        __m128i Pi = _mm_cvtpd_epi32(Pid);                                                          // Pi   = | [x] : [y] : 0 : 0 | as integers
        __m128d Pf00 = _mm_sub_pd(P, Pid);                                                          // Pf00 = | {x} : {y} | as doubles

        Pi = _mm_unpacklo_epi32(Pi, Pi);                                                            // Pi   = | [x] : [x] : [y] : [y] |
        __m128i M2x2 = _mm_load_si128((const __m128i*) glm::value_ptr(hash_mat2x2));                // M2x2 = | m00 : m10 : m01 : m11 | = | hash_X : hash_Y |
        __m128i hash2d_00_00 = _mm_mullo_epi32(Pi, M2x2);                                           // hash2d_00_00 = | m00 : m10 : m01 : m11 | = | m00 * [x] : m10 * [x] : m01 * [y] : m11 * [y] |
        __m128i _hash0_hashY = _mm_slli_si128 (_mm_srli_si128(M2x2, 8), 8);                         // _hash0_hashY = |    0   : hash_Y |
        __m128i _hashX_hashX = _mm_unpacklo_epi64 (M2x2, M2x2);                                     // _hashX_hashX = | hash_X : hash_X |
        hash2d_00_00 = _mm_add_epi32(hash2d_00_00, _mm_shuffle_epi32(hash2d_00_00, _MM_SHUFFLE(1,0,3,2)));  
                                                                                                    // hash2d_00_00 = | hash00x : hash00y : hash00x : hash00y |
        __m128i hash2d_00_01 = _mm_add_epi32(hash2d_00_00, _hash0_hashY);                           // hash2d_00_01 = | hash00x : hash00y : hash01x : hash01y |
        __m128i hash2d_10_11 = _mm_add_epi32(hash2d_00_01, _hashX_hashX);                           // hash2d_10_11 = | hash10x : hash10y : hash11x : hash11y |

        __m128i hash_factor = _mm_set1_epi32(HASH_FACTOR);
        hash2d_00_01 = _mm_mullo_epi32(_mm_xor_si128(hash2d_00_01, _mm_srli_epi32(hash2d_00_01, HASH_SHIFT)), hash_factor);
        hash2d_10_11 = _mm_mullo_epi32(_mm_xor_si128(hash2d_10_11, _mm_srli_epi32(hash2d_10_11, HASH_SHIFT)), hash_factor);

        __m128d grad00 = _mm_cvtepi32_pd(hash2d_00_01);
        __m128d grad01 = _mm_cvtepi32_pd(_mm_unpackhi_epi64(hash2d_00_01, hash2d_00_01));
        __m128d grad10 = _mm_cvtepi32_pd(hash2d_10_11);
        __m128d grad11 = _mm_cvtepi32_pd(_mm_unpackhi_epi64(hash2d_10_11, hash2d_10_11));

        __m128d Pf11 = _mm_sub_pd(Pf00, _m128d_1);                                                  // Pf11 = | {x} - 1 : {y} - 1 |
        __m128d Pf01 = _mm_shuffle_pd(Pf00, Pf11, _MM_SHUFFLE2(1, 0));                              // Pf01 = | {x} : {y} - 1 |
        __m128d Pf10 = _mm_shuffle_pd(Pf11, Pf00, _MM_SHUFFLE2(1, 0));                              // Pf10 = | {x} - 1 : {y} |

        __m128d dot00_10 = _mm_unpacklo_pd(_mm_dp_pd(grad00, Pf00, 0x31), _mm_dp_pd(grad10, Pf10, 0x31));
        __m128d dot01_11 = _mm_unpacklo_pd(_mm_dp_pd(grad01, Pf01, 0x31), _mm_dp_pd(grad11, Pf11, 0x31));

        __m128d Ps = smooth_step5(Pf00);
        __m128d dot_x01 = _mm_add_pd(dot00_10, _mm_mul_pd(_mm_sub_pd(dot01_11, dot00_10), _mm_unpackhi_pd(Ps, Ps)));
        __m128d val = _mm_add_sd(dot_x01, _mm_mul_sd(_mm_sub_sd(_mm_unpackhi_pd(dot_x01, dot_x01), dot_x01), Ps));
        return _mm_cvtsd_f64(val) * GRADIENT_2D_SSE41_NORMALIZATION_FACTOR;
    }

template<> float gradient_sse41(const glm::vec3& P)
{
    return 0.0f;
}

template<> double gradient_sse41(const glm::dvec3& P)
{
    return 0.0;
}

template<> float gradient_sse41(const glm::vec4& P)
{
    return 0.0f;
}

    template<> double gradient_sse41(const glm::dvec4& P)
    {
    }

} // namespace noise
