#include <glm/gtx/norm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "avx2.hpp"
#include "common.hpp"
#include "simdhelp.hpp"

#define _MM_SHUFFLE_MASK2(a0,a1) ((a0 & 0x3) | ((a1 << 2) & 0xC))
#define _MM_SHUFFLE_MASK4(a0,a1,a2,a3) (_MM_SHUFFLE_MASK2(a0, a1) | (_MM_SHUFFLE_MASK2(a2, a3) << 4))
#define _MM_BLEND_MASK2(a0,a1) (((a0 << 0) & 0x1) | ((a1 << 1) & 0x2))
#define _MM_BLEND_MASK4(a0,a1,a2,a3) (_MM_BLEND_MASK2(a0, a1) | (_MM_BLEND_MASK2(a2, a3) << 2))
#define _MM_BLEND_MASK8(a0,a1,a2,a3,a4,a5,a6,a7) (_MM_BLEND_MASK4(a0,a1,a2,a3) | (_MM_BLEND_MASK4(a4,a5,a6,a7) << 4))

namespace noise {



//=======================================================================================================================================================================================================================
// AVX 2 implementation of value noise                                                                                                                                                                                   
//=======================================================================================================================================================================================================================
template<> float value_avx2<float>(const glm::vec2& P) 
{
    return 0.0f;
}


    template<> double value_avx2<double>(const glm::dvec2& arg) 
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
        hash = _mm_mullo_epi32(hash, hash_factor);                                                      // hash = | hash(0, 0) : hash(0, 1) : hash(1, 0) : hash(1, 1) |

        __m256d value = _mm256_cvtepi32_pd(hash);
        __m128d val_00_01 = _mm256_castpd256_pd128(value);
        __m128d val_10_11 = _mm256_extractf128_pd(value, 1);
        __m128d val_01 = _mm_fmadd_pd(_mm_sub_pd(val_10_11, val_00_01), _mm_unpackhi_pd(Ps, Ps), val_00_01);
        __m128d val = _mm_fmadd_sd(_mm_sub_sd(_mm_unpackhi_pd(val_01, val_01), val_01), Ps, val_01);

        return _mm_cvtsd_f64(val) * VALUE_2D_AVX2_NORMALIZATION_FACTOR;
    }

template<> float value_avx2(const glm::vec3& P)
{
    return 0.0f;
}

template<> double value_avx2(const glm::dvec3& P)
{
    return 0.0;
}

template<> float value_avx2(const glm::vec4& P)
{
    return 0.0f;
}


    template<> double value_avx2(const glm::dvec4& arg)
    {
        __m256d P = _mm256_loadu_pd(glm::value_ptr(arg));                                               // P   = |  x  :  y  :  z  :  w  |
        __m256d Pid = _mm256_floor_pd(P);                                                               // Pid = | [x] : [y] : [z] : [w] | as doubles
        __m128i Pi = _mm256_cvtpd_epi32(Pid);                                                           // Pi  = | [x] : [y] : [z] : [w] | as integers
        __m256d Pf = _mm256_sub_pd(P, Pid);                                                             // Pf  = | {x} : {y} : {z} : {w} |

        __m128i Pm = _mm_mullo_epi32(Pi, _m128i_LINEAR_FACTOR_XYZW);                                    // Pm  = | [x] * F_X : [y] * F_Y : [z] * F_Z : [w] * F_W |
        Pm = _mm_hadd_epi32(Pm, Pm);
        Pm = _mm_hadd_epi32(Pm, Pm);
        __m128i Pm00 = _mm_add_epi32(Pm, _m128i_LINEAR_SHIFT_0_X_Y_XY);                                 // | L(0,0,0,0) : L(1,0,0,0) : L(0,1,0,0) : L(1,1,0,0) |

        __m128i Fzzzz = _mm_shuffle_epi32(_m128i_LINEAR_FACTOR_XYZW, _MM_SHUFFLE_MASK4(2,2,2,2));       // | F_Z : F_Z : F_Z : F_Z |
        __m128i Fwwww = _mm_shuffle_epi32(_m128i_LINEAR_FACTOR_XYZW, _MM_SHUFFLE_MASK4(3,3,3,3));       // | F_W : F_W : F_W : F_W |

        __m128i Pm10 = _mm_add_epi32(Pm00, Fzzzz);                                                      // | L(0,0,1,0) : L(1,0,1,0) : L(0,1,1,0) : L(1,1,1,0) |
        __m128i Pm01 = _mm_add_epi32(Pm00, Fwwww);                                                      // | L(0,0,0,1) : L(1,0,0,1) : L(0,1,0,1) : L(1,1,0,1) |
        __m128i Pm11 = _mm_add_epi32(Pm10, Fwwww);                                                      // | L(0,0,1,1) : L(1,0,1,1) : L(0,1,1,1) : L(1,1,1,1) |

        __m128i hash_factor = _mm_set1_epi32(HASH_FACTOR);

        Pm00 = _mm_mullo_epi32(_mm_xor_si128(Pm00, _mm_srli_epi32(Pm00, HASH_SHIFT)), hash_factor);     // | hash (0,0,0,0) : hash (1,0,0,0) : hash (0,1,0,0) : hash (1,1,0,0) |
        Pm10 = _mm_mullo_epi32(_mm_xor_si128(Pm10, _mm_srli_epi32(Pm10, HASH_SHIFT)), hash_factor);     // | hash (0,0,1,0) : hash (1,0,1,0) : hash (0,1,1,0) : hash (1,1,1,0) |
        Pm01 = _mm_mullo_epi32(_mm_xor_si128(Pm01, _mm_srli_epi32(Pm01, HASH_SHIFT)), hash_factor);     // | hash (0,0,0,1) : hash (1,0,0,1) : hash (0,1,0,1) : hash (1,1,0,1) |
        Pm11 = _mm_mullo_epi32(_mm_xor_si128(Pm11, _mm_srli_epi32(Pm11, HASH_SHIFT)), hash_factor);     // | hash (0,0,1,1) : hash (1,0,1,1) : hash (0,1,1,1) : hash (1,1,1,1) |

        __m256d Ps = smooth_step5(Pf);                                                                  // Ps   = | s_x : s_y : s_z : s_w |

        // interpolate with respect to w-component for z = 1 :: Pm11 and Pm10 values
        __m256d val_zw10 = _mm256_cvtepi32_pd(Pm10);                                                    // | hash (0,0,1,0) : hash (1,0,1,0) : hash (0,1,1,0) : hash (1,1,1,0) | as doubles
        __m256d val_zw11 = _mm256_cvtepi32_pd(Pm11);                                                    // | hash (0,0,1,1) : hash (1,0,1,1) : hash (0,1,1,1) : hash (1,1,1,1) | as doubles

        __m256d ws_ws_ws_ws = _mm256_permute4x64_pd(Ps, _MM_SHUFFLE_MASK4(3,3,3,3));                    // ws_ws_ws_ws = | ws : ws : ws : ws |
        __m256d val_z1 = _mm256_fmadd_pd(_mm256_sub_pd(val_zw11, val_zw10), ws_ws_ws_ws, val_zw10);

        // interpolate with respect to w-component for z = 0 :: Pm01 and Pm00 values
        __m256d val_zw00 = _mm256_cvtepi32_pd(Pm00);                                                    // | hash (0,0,0,0) : hash (1,0,0,0) : hash (0,1,0,0) : hash (1,1,0,0) | as doubles
        __m256d val_zw01 = _mm256_cvtepi32_pd(Pm01);                                                    // | hash (0,0,0,1) : hash (1,0,0,1) : hash (0,1,0,1) : hash (1,1,0,1) | as doubles

        __m256d val_z0 = _mm256_fmadd_pd(_mm256_sub_pd(val_zw01, val_zw00), ws_ws_ws_ws, val_zw00);

        // interpolate with respect to z-component
        __m256d zs_zs_zs_zs = _mm256_permute4x64_pd(Ps, _MM_SHUFFLE_MASK4(2,2,2,2));                    // zs_zs_zs_zs = | zs : zs : zs : zs |
        __m256d val_xy = _mm256_fmadd_pd(_mm256_sub_pd(val_z1, val_z0), zs_zs_zs_zs, val_z0);

        __m128d val_y0 = _mm256_castpd256_pd128(val_xy);                     
        __m128d val_y1 = _mm256_extractf128_pd(val_xy, 1);
        __m128d Pxys = _mm256_castpd256_pd128(Ps);
        __m128d val_x01 = _mm_fmadd_pd(_mm_sub_pd(val_y1, val_y0), _mm_unpackhi_pd(Pxys, Pxys), val_y0);
        __m128d val = _mm_fmadd_sd(_mm_sub_sd(_mm_unpackhi_pd(val_x01, val_x01), val_x01), _mm_unpacklo_pd(Pxys, Pxys), val_x01);

        return _mm_cvtsd_f64(val) * VALUE_4D_AVX2_NORMALIZATION_FACTOR;
    }
              
    const double VALUE_2D_FBM4_AVX2_NORMALIZATION_FACTOR = 1.0;

    double value_fbm4_avx2(const glm::dvec2& point, const glm::dmat2* matrices, const glm::dvec2* shifts, const glm::dvec4& amplitudes)
    {
        __m128d P = _mm_load_pd(glm::value_ptr(point));                                                 // P   = | x_0 : y_0 |
        __m256d M0 = _mm256_load_pd(glm::value_ptr(matrices[0]));                                       // M0  = | m0_00 : m0_10 : m0_01 : m0_11 |
        __m256d M1 = _mm256_load_pd(glm::value_ptr(matrices[1]));                                       // M1  = | m1_00 : m1_10 : m1_01 : m1_11 |
        __m256d M2 = _mm256_load_pd(glm::value_ptr(matrices[2]));                                       // M2  = | m2_00 : m2_10 : m2_01 : m2_11 |
        __m256d M3 = _mm256_load_pd(glm::value_ptr(matrices[3]));                                       // M3  = | m3_00 : m3_10 : m3_01 : m3_11 |

        __m256d S01 = _mm256_load_pd(glm::value_ptr(shifts[0]));                                        // S01 = | s0x : s0y : s1x : s1y |
        __m256d S23 = _mm256_load_pd(glm::value_ptr(shifts[2]));                                        // S23 = | s2x : s2y : s3x : s3y |

        __m256d PP = _mm256_insertf128_pd(_mm256_castpd128_pd256(P), P, 1);                             // PP  = | x_0 : y_0 : x_0 : y_0 |

        __m256d MP0 = _mm256_mul_pd(M0, PP);                                                            // MP0 = | m0_00 * x_0 : m0_10 * y_0 : m0_01 * x_0 : m0_11 * y_0 | 
        __m256d MP1 = _mm256_mul_pd(M1, PP);                                                            // MP1 = | m1_00 * x_0 : m1_10 * y_0 : m1_01 * x_0 : m1_11 * y_0 | 
        __m256d MP2 = _mm256_mul_pd(M2, PP);                                                            // MP2 = | m2_00 * x_0 : m2_10 * y_0 : m2_01 * x_0 : m2_11 * y_0 | 
        __m256d MP3 = _mm256_mul_pd(M3, PP);                                                            // MP3 = | m3_00 * x_0 : m3_10 * y_0 : m3_01 * x_0 : m3_11 * y_0 | 

        MP0 = _mm256_hadd_pd(MP0, MP1);                                                                 // MP0 = | m0_00 * x_0 + m0_10 * y_0 : m1_00 * x_0 + m1_10 * y_0 : m0_01 * x_0 + m0_11 * y_0 : m1_01 * x_0 + m1_11 * y_0 | 
       MP2 = _mm256_hadd_pd(MP2, MP3);                                                                 // MP2 = | m2_00 * x_0 + m2_10 * y_0 : m3_00 * x_0 + m3_10 * y_0 : m2_01 * x_0 + m2_11 * y_0 : m3_01 * x_0 + m3_11 * y_0 | 

        MP0 = _mm256_add_pd(MP0, S01);                                                                  // MP0 = | x_0 : x_1 : y_0 : y_1 |
        MP2 = _mm256_add_pd(MP2, S23);                                                                  // MP2 = | x_2 : x_3 : y_2 : y_3 |

        __m256d P01id = _mm256_floor_pd(MP0);                                                           // P01id = | [x_0] : [x_1] : [y_0] : [y_1] | - doubles
        __m256d P23id = _mm256_floor_pd(MP2);                                                           // P23id = | [x_2] : [x_3] : [y_2] : [y_3] | - doubles

        __m256d P01f = _mm256_sub_pd(MP0, P01id);                                                       // P01f  = | {x_0} : {x_1} : {y_0} : {y_1} |
        __m256d P23f = _mm256_sub_pd(MP2, P23id);                                                       // P23f  = | {x_2} : {x_3} : {y_2} : {y_3} |

        __m128i P01i = _mm256_cvtpd_epi32(P01id);                                                       // P01id = | [x_0] : [x_1] : [y_0] : [y_1] | - int32
        __m128i P23i = _mm256_cvtpd_epi32(P23id);                                                       // P23id = | [x_2] : [x_3] : [y_2] : [y_3] | - int32

        __m128i PXi = _mm_unpacklo_epi64(P01i, P23i);                                                   // PXi   = | [x_0] : [x_1] : [x_2] : [x_3] |
        __m128i PYi = _mm_unpackhi_epi64(P01i, P23i);                                                   // PYi   = | [y_0] : [y_1] : [y_2] : [y_3] |

        __m128i XXXX = _mm_set1_epi32(HASH_FACTOR_X);
        __m128i YYYY = _mm_set1_epi32(HASH_FACTOR_Y);

        PXi = _mm_mullo_epi32(PXi, XXXX);                                         
        PYi = _mm_mullo_epi32(PYi, YYYY);                                         
        
        __m128i hash00 = _mm_add_epi32(PXi, PYi);
        __m128i hash01 = _mm_add_epi32(hash00, YYYY);
        __m128i hash10 = _mm_add_epi32(hash00, XXXX);
        __m128i hash11 = _mm_add_epi32(hash01, XXXX);

        hash00 = _mm_xor_si128(hash00, _mm_srli_epi32(hash00, HASH_SHIFT));
        hash01 = _mm_xor_si128(hash01, _mm_srli_epi32(hash01, HASH_SHIFT));
        hash10 = _mm_xor_si128(hash10, _mm_srli_epi32(hash10, HASH_SHIFT));
        hash11 = _mm_xor_si128(hash11, _mm_srli_epi32(hash11, HASH_SHIFT));
        hash00 = _mm_mullo_epi32(hash00, _m128i_HASH_FACTOR);
        hash01 = _mm_mullo_epi32(hash01, _m128i_HASH_FACTOR);
        hash10 = _mm_mullo_epi32(hash10, _m128i_HASH_FACTOR);
        hash11 = _mm_mullo_epi32(hash11, _m128i_HASH_FACTOR);

        __m256d val00 = _mm256_cvtepi32_pd(hash00);                                                     
        __m256d val01 = _mm256_cvtepi32_pd(hash01);
        __m256d val10 = _mm256_cvtepi32_pd(hash10);
        __m256d val11 = _mm256_cvtepi32_pd(hash11);

        __m256d P01s = smooth_step5(P01f);                                                              // smoothing factors | [xs_0] : [xs_1] : [ys_0] : [ys_1] |
        __m256d P23s = smooth_step5(P23f);                                                              //                   | [xs_2] : [xs_3] : [ys_2] : [ys_3] |

        __m256d PXs = _mm256_permute2f128_pd(P01s, P23s, 0x20);                                         // | [xs_0] : [xs_1] : [xs_2] : [xs_3] |
        __m256d PYs = _mm256_permute2f128_pd(P01s, P23s, 0x33);                                         // | [ys_0] : [ys_1] : [ys_2] : [ys_3] |



        __m256d valY0 = _mm256_add_pd(val00, _mm256_mul_pd(PXs, _mm256_sub_pd(val10, val00)));          // to be multiplied by 
        __m256d valY1 = _mm256_add_pd(val01, _mm256_mul_pd(PXs, _mm256_sub_pd(val11, val01)));
        __m256d val = _mm256_add_pd(valY0, _mm256_mul_pd(PYs, _mm256_sub_pd(valY1, valY0)));


        __m256d amp = _mm256_load_pd(glm::value_ptr(amplitudes));
        val = _mm256_mul_pd(val, amp);

        val = _mm256_hadd_pd(val, val);
        val = _mm256_hadd_pd(val, val);

        return _mm_cvtsd_f64(_mm256_castpd256_pd128(val)) * VALUE_2D_FBM4_AVX2_NORMALIZATION_FACTOR;
    }
                                                                                                                                                                                                           
//=======================================================================================================================================================================================================================
// AVX 2 implementation of simplex noise                                                                                                                                                                                 
//=======================================================================================================================================================================================================================
template<> float simplex_avx2<float>(const glm::vec2& P) 
{
    return 0.0f;
}


    template<> double simplex_avx2<double>(const glm::dvec2& arg) 
    {
        __m128d __m128d_F2_F2 = _mm_unpacklo_pd(__m128d_F2_HALF, __m128d_F2_HALF);
        __m128d __m128d_G2_G2 = _mm_unpacklo_pd(__m128d_G2_G2M1, __m128d_G2_G2M1);
        __m128d __m128d_G2M1_G2 = _mm_shuffle_pd(__m128d_G2_G2M1, __m128d_G2_G2M1, _MM_SHUFFLE2(0, 1));
        __m256d __m256d_G2M1_G2_G2M1_G2 = _mm256_insertf128_pd(_mm256_castpd128_pd256(__m128d_G2M1_G2), __m128d_G2M1_G2, 1);

        __m128d P = _mm_loadu_pd(glm::value_ptr(arg));                                              // P     = | x : y |
        __m128d t = _mm_dp_pd(P, __m128d_F2_F2, 0x33);                                              // t     = | (P.x + P.y) * F2 : (P.x + P.y) * F2 |
        __m128d Pid = _mm_floor_pd(_mm_add_pd(P, t));                                               // Pid   = | [P.x + (P.x + P.y) * F2] : [P.y + (P.x + P.y) * F2] |
        __m128i Pi = _mm_cvtpd_epi32(Pid);                                                          // Pi    = | [P.x + (P.x + P.y) * F2] : [P.y + (P.x + P.y) * F2] : 0 : 0 | as integers
        __m128d t_inv = _mm_dp_pd(Pid, __m128d_G2_G2, 0x33);                                        // t_inv = | (Pi.x + Pi.y) * G2 : (Pi.x + Pi.y) * G2 |
        __m128d P0 = _mm_sub_pd(Pid, t_inv);                                                        // base of the triangular cell containing P

        __m128d Pf00 = _mm_sub_pd(P, P0);                                                           // | Pf00x : Pf00y |
        __m128d Pf01 = _mm_add_pd(Pf00, __m128d_G2_G2M1);                                           // | Pf01x : Pf01y |
        __m256d Pf00_01 = _mm256_insertf128_pd(_mm256_castpd128_pd256(Pf00), Pf01, 1);              // | Pf00x : Pf00y : Pf01x : Pf01y |
        __m256d Pf10_11 = _mm256_add_pd(Pf00_01, __m256d_G2M1_G2_G2M1_G2);                          // | Pf10x : Pf10y : Pf11x : Pf11y |

        Pi = _mm_unpacklo_epi32(Pi, Pi);                                                            // Pi   = | [x] : [x] : [y] : [y] |
        __m128i M2x2 = _mm_load_si128((const __m128i*) glm::value_ptr(hash_mat2x2));                // M2x2 = | m00 : m10 : m01 : m11 | = | hash_X : hash_Y |

        __m128i hash2d_00_00 = _mm_mullo_epi32(Pi, M2x2);                                           // hash2d_00_00 = | m00 : m10 : m01 : m11 | = | m00 * [x] : m10 * [x] : m01 * [y] : m11 * [y] |
        __m128i _hash0_hashY = _mm_slli_si128 (_mm_srli_si128(M2x2, 8), 8);                         // _hash0_hashY = |    0   : hash_Y |
        __m128i _hashX_hashX = _mm_unpacklo_epi64 (M2x2, M2x2);                                     // _hashX_hashX = | hash_X : hash_X |
        hash2d_00_00 = _mm_add_epi32(hash2d_00_00, _mm_shuffle_epi32(hash2d_00_00, _MM_SHUFFLE(1,0,3,2)));  
                                                                                                    // hash2d_00_00 = | hash00x : hash00y : hash00x : hash00y |
        __m128i hash2d_00_01 = _mm_add_epi32(hash2d_00_00, _hash0_hashY);                           // hash2d_00_01 = | hash00x : hash00y : hash01x : hash01y |
        __m128i hash2d_10_11 = _mm_add_epi32(hash2d_00_01, _hashX_hashX);                           // hash2d_10_11 = | hash10x : hash10y : hash11x : hash11y |

        __m128i _m128i_HASH_FACTOR = _mm_set1_epi32(HASH_FACTOR);                                                                    
        hash2d_00_01 = _mm_mullo_epi32(_mm_xor_si128(hash2d_00_01, _mm_srli_epi32(hash2d_00_01, HASH_SHIFT)), _m128i_HASH_FACTOR);
        hash2d_10_11 = _mm_mullo_epi32(_mm_xor_si128(hash2d_10_11, _mm_srli_epi32(hash2d_10_11, HASH_SHIFT)), _m128i_HASH_FACTOR);

        __m256d grad00_01 = _mm256_cvtepi32_pd(hash2d_00_01);
        __m256d grad10_11 = _mm256_cvtepi32_pd(hash2d_10_11);

        __m256d dot = _mm256_hadd_pd(_mm256_mul_pd(grad00_01, Pf00_01), _mm256_mul_pd(grad10_11, Pf10_11));
        __m256d len = _mm256_hadd_pd(_mm256_mul_pd(Pf00_01, Pf00_01), _mm256_mul_pd(Pf10_11, Pf10_11));
        __m256d zero = _mm256_setzero_pd();
        __m256d half = _mm256_permute4x64_pd(_mm256_castpd128_pd256(__m128d_F2_HALF), _MM_SHUFFLE(1,1,1,1));

        len = _mm256_sub_pd(half, len);
        len = _mm256_max_pd(len, zero);
        len = _mm256_mul_pd(len, len);
        len = _mm256_mul_pd(len, dot);

        __m128d val_00_10 = _mm256_castpd256_pd128(len);
        __m128d val_01_11 = _mm256_extractf128_pd(len, 1);
        __m128d val_01 = _mm_add_pd(val_00_10, val_01_11);
        __m128d val_x1 = _mm_unpackhi_pd(val_01, val_01);
        __m128d val = _mm_add_sd(val_01, val_x1);
        return _mm_cvtsd_f64(val) * SIMPLEX_2D_AVX2_NORMALIZATION_FACTOR;      
    }

template<> float simplex_avx2(const glm::vec3& P)
{
    return 0.0f;
}


    template<> double simplex_avx2<double>(const glm::dvec3& P)
    {
        return 0.0; 
/*
        static const double F3 = 1.0 / 3.0;
        static const double G3_1 = 1.0 / 6.0;
        static const double G3_2 = 2.0 * G3_1;
        static const double G3_3 = 3.0 * G3_1 - 1.0;

        __m128d Pxy = _mm_loadu_pd(&arg.x);                                                             // Pxy = |  x  :  y  |
        __m128d Pz0 = _mm_load_sd(&arg.z);                                                              // Pz0 = |  z  :  0  |
        __m256d P = _mm256_insertf128_pd(_mm256_castpd128_pd256(Pxy), Pz0, 1);                          // P   = |  x  :  y  :  z  : 0 |

        __m128d s = _mm_add_pd(Pxy, Pz0);
        s = _mm_hadd_pd(s, s);                                                                          // s = | x + y + z : x + y + z |
//        __m256d Pid = _mm256_floor_pd(P);                                                             // Pid = |  x  :  y  :  z  : 0 | --- doubles
//        __m256d Pf = _mm256_sub_pd(P, Pid);                                                           // Pf  = | {x} : {y} : {z} : 0 |
//        __m128i Pi = _mm256_cvtpd_epi32(Pid);                                                         // Pi =  | [x] : [y] : [z] : 0 | --- integers

        double t = (P.x + P.y + P.z) * F3;
        glm::ivec3 Pi = entier(P + glm::dvec3(t));
        t = (Pi.x + Pi.y + Pi.z) * G3_1;

        glm::dvec3 P0 = glm::dvec3(Pi) - glm::dvec3(t);
        glm::dvec3 p0 = P - P0;
        glm::dvec3 p1 = p0 + glm::dvec3(G3_1);
        glm::dvec3 p2 = p0 + glm::dvec3(G3_2);
        glm::dvec3 p3 = p0 + glm::dvec3(G3_3);

        glm::ivec3 hash0 = hash_mat3x3 * Pi;
        glm::ivec3 hash1, hash2;
        glm::ivec3 hash3 = hash0 + hashX_vec3 + hashY_vec3 + hashZ_vec3;

      





        bool zx = p0.z < p0.x;
        bool xy = p0.x < p0.y;
        bool yz = p0.y < p0.z;


        __m256d p0_zxy = _mm256_permute4x64_pd(p0, _MM_SHUFFLE_MASK4(2,0,1,3));

        __m256d zx_xy_yz = _mm256_cmplt_pd(p0_zxy, p0);
        __m256d xy_yz_zx = _mm256_permute4x64_pd(zx_xy_yz, _MM_SHUFFLE_MASK4(2,0,1,3));

        __m256d i1_j1_k1 = _mm256_andnot_pd(zx_xy_yz, xy_yz_zx);

        int i0 = 0;
        int j0 = 0;
        int k0 = 0;

        int i1 = zx && !xy;    // x - maximal
        int j1 = xy && !yz;    // y - maximal
        int k1 = yz && !zx;    // z - maximal

        
        int i2 = zx || !xy;    // x - not minimal
        int j2 = xy || !yz;    // y - not minimal
        int k2 = yz || !zx;    // z - not minimal

        hash1 = hash0 + i1 * hashX_vec3 + j1 * hashY_vec3 + k1 * hashZ_vec3;
        hash2 = hash0 + i2 * hashX_vec3 + j2 * hashY_vec3 + k2 * hashZ_vec3;





        hash0 = hash0 +  0 * hashX_vec3 +  0 * hashY_vec3 +  0 * hashZ_vec3;
        hash1 = hash0 + i1 * hashX_vec3 + j1 * hashY_vec3 + k1 * hashZ_vec3;
        hash2 = hash0 + i2 * hashX_vec3 + j2 * hashY_vec3 + k2 * hashZ_vec3;
        hash3 = hash0 +  1 * hashX_vec3 +  1 * hashY_vec3 +  1 * hashZ_vec3;

        | hash0 : hash1 : hash2 : hash3 |

        p0 = | p0.x : p0.y : p0.z : 0 |
        p1 = | p1.x : p1.y : p1.z : 0 |
        p2 = | p2.x : p2.y : p2.z : 0 |
        p3 = | p3.x : p3.y : p3.z : 0 |


        _mm256_hadd_pd(p0, p2); | p0.x + p0.y : p2.x + p2.y : p0.z : p2.z |


        int i3 = 1;
        int j3 = 1;
        int k3 = 1;

        hash0 = hash(hash0);
        hash1 = hash(hash1);
        hash2 = hash(hash2);
        hash3 = hash(hash3);

        p1 = p1 - glm::dvec3(i1, j1, k1);
        p2 = p2 - glm::dvec3(i2, j2, k2);

        double t0 = 0.6 - glm::length2(p0); if (t0 < 0.0) t0 = 0.0; t0 *= t0;
        double t1 = 0.6 - glm::length2(p1); if (t1 < 0.0) t1 = 0.0; t1 *= t1;
        double t2 = 0.6 - glm::length2(p2); if (t2 < 0.0) t2 = 0.0; t2 *= t2;
        double t3 = 0.6 - glm::length2(p3); if (t3 < 0.0) t3 = 0.0; t3 *= t3;

        
        double n0 = t0 * t0 * glm::dot(p0, glm::dvec3(hash0));
        double n1 = t1 * t1 * glm::dot(p1, glm::dvec3(hash1));
        double n2 = t2 * t2 * glm::dot(p2, glm::dvec3(hash2));
        double n3 = t3 * t3 * glm::dot(p3, glm::dvec3(hash3));
        
        return 0.00000001197882587368369716265345 * (n0 + n1 + n2 + n3);
*/
    }



template<> float simplex_avx2(const glm::vec4& P)
{
    return 0.0f;
}

template<> double simplex_avx2(const glm::dvec4& P)
{
    return 0.0;
}
                                                                                                                                                                                                                         
//=======================================================================================================================================================================================================================
// AVX 2 implementation of gradient noise                                                                                                                                                                                
//=======================================================================================================================================================================================================================
template<> float gradient_avx2<float>(const glm::vec2& P) 
{
    return 0.0f;
}

    template<> double gradient_avx2<double>(const glm::dvec2& arg) 
    {
        __m128d P = _mm_load_pd(glm::value_ptr(arg));
        __m128d Pid = _mm_floor_pd(P);                                                              // Pid  = | [x] : [y] | as doubles
        __m128i Pi = _mm_cvtpd_epi32(Pid);                                                          // Pi   = | [x] : [y] : 0 : 0 | as integers
        __m128d Pf0 = _mm_sub_pd(P, Pid);                                                           // Pf0  = | {x} : {y} | as doubles
        __m128d _m128d_one = _mm_set1_pd(1.0);
        __m128d Pf1 = _mm_sub_pd(Pf0, _m128d_one);                                                  // Pf0  = | {x} - 1 : {y} - 1 |
        __m128d Ps = smooth_step5(Pf0);

        __m256d Pf0Pf0 = _mm256_insertf128_pd(_mm256_castpd128_pd256(Pf0), Pf0, 1);
        __m256d Pf1Pf1 = _mm256_insertf128_pd(_mm256_castpd128_pd256(Pf1), Pf1, 1);

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

        __m256d grad2d_00_01 = _mm256_cvtepi32_pd(hash2d_00_01);                                    // grad2d_00_01 = | grad00x : grad00y : grad01x : grad01y | as doubles
        __m256d grad2d_10_11 = _mm256_cvtepi32_pd(hash2d_10_11);                                    // grad2d_10_11 = | grad10x : grad10y : grad11x : grad11y | as doubles

        __m256d dp_00_01 = _mm256_blend_pd(Pf0Pf0, Pf1Pf1, _MM_BLEND_MASK4(0, 0, 0, 1));
        __m256d dp_10_11 = _mm256_blend_pd(Pf0Pf0, Pf1Pf1, _MM_BLEND_MASK4(1, 0, 1, 1));

        __m256d val_xy = _mm256_hadd_pd(
                            _mm256_mul_pd(dp_00_01, grad2d_00_01), 
                            _mm256_mul_pd(dp_10_11, grad2d_10_11)
                         );                                                                         // | 00 : 10 : 01 : 11 |

        __m128d val_00_10 = _mm256_castpd256_pd128(val_xy);
        __m128d val_01_11 = _mm256_extractf128_pd(val_xy, 1);
        __m128d val_01 = _mm_fmadd_pd(_mm_sub_pd(val_01_11, val_00_10), _mm_unpackhi_pd(Ps, Ps), val_00_10);
        __m128d val_x1 = _mm_unpackhi_pd(val_01, val_01);
        __m128d val = _mm_fmadd_sd(_mm_sub_pd(val_x1, val_01), Ps, val_01);

        return GRADIENT_2D_AVX2_NORMALIZATION_FACTOR * _mm_cvtsd_f64(val);
    }

template<> float gradient_avx2(const glm::vec3& P)
{
    return 0.0f;
}

template<> double gradient_avx2(const glm::dvec3& P)
{
    return 0.0;
}

    template<> float gradient_avx2(const glm::vec4& arg)
    {
        __m128 P = _mm_loadu_ps(glm::value_ptr(arg));                                               // P    = |  x  :  y  :  z  :  w  |
        __m128 Pif = _mm_floor_ps(P);                                                               // Pid  = | [x] : [y] : [z] : [w] | as floats
        __m128i Pi = _mm_cvtps_epi32(Pif);                                                          // Pi   = | [x] : [y] : [z] : [w] | as integers
        __m128 Pf0 = _mm_sub_ps(P, Pif);                                                            // Pf0  = | {x} : {y} : {z} : {w} |
        __m128 Ps = smooth_step5(Pf0);                                                              // Ps0  = | s_x : s_y : s_z : s_w |

        const float one = 1.0f;
        __m128 _m128_one = _mm_broadcast_ss(&one);
        __m128 Pf1 = _mm_sub_ps(Pf0, _m128_one);                                                    // Pf0  = | {x} - 1 : {y} - 1 : {z} - 1 : {w} - 1 |

        __m128i hash_mat_X = _mm_load_si128((const __m128i*)glm::value_ptr(hash_mat4x4[0]));        // X - column of the hash matrix
        __m128i hash_mat_Y = _mm_load_si128((const __m128i*)glm::value_ptr(hash_mat4x4[1]));        // Y - column of the hash matrix
        __m128i hash_mat_Z = _mm_load_si128((const __m128i*)glm::value_ptr(hash_mat4x4[2]));        // Z - column of the hash matrix
        __m128i hash_mat_W = _mm_load_si128((const __m128i*)glm::value_ptr(hash_mat4x4[3]));        // W - column of the hash matrix

        __m128i hash0000 = _mm_add_epi32(
                                _mm_add_epi32(
                                    _mm_mullo_epi32(hash_mat_X, _mm_shuffle_epi32(Pi, _MM_SHUFFLE(0,0,0,0))),
                                    _mm_mullo_epi32(hash_mat_Y, _mm_shuffle_epi32(Pi, _MM_SHUFFLE(1,1,1,1)))
                                ),
                                _mm_add_epi32(
                                    _mm_mullo_epi32(hash_mat_Z, _mm_shuffle_epi32(Pi, _MM_SHUFFLE(2,2,2,2))),
                                    _mm_mullo_epi32(hash_mat_W, _mm_shuffle_epi32(Pi, _MM_SHUFFLE(3,3,3,3)))
                                )
                            );

        __m128i hash1000 = _mm_add_epi32(hash0000, hash_mat_X);

        __m256i hash4d_0000_1000 = _mm256_inserti128_si256(_mm256_castsi128_si256(hash0000), hash1000, 1);
        __m256i hash4d_0100_1100 = _mm256_add_epi32(hash4d_0000_1000, _mm256_broadcastsi128_si256(hash_mat_Y));

        __m256i hash_mat_ZZ = _mm256_broadcastsi128_si256(hash_mat_Z); 
        __m256i hash_mat_WW = _mm256_broadcastsi128_si256(hash_mat_W); 

        __m256i hash4d_0010_1010 = _mm256_add_epi32(hash4d_0000_1000, hash_mat_ZZ);
        __m256i hash4d_0110_1110 = _mm256_add_epi32(hash4d_0100_1100, hash_mat_ZZ);
        __m256i hash4d_0001_1001 = _mm256_add_epi32(hash4d_0000_1000, hash_mat_WW);
        __m256i hash4d_0101_1101 = _mm256_add_epi32(hash4d_0100_1100, hash_mat_WW);
        __m256i hash4d_0011_1011 = _mm256_add_epi32(hash4d_0010_1010, hash_mat_WW);
        __m256i hash4d_0111_1111 = _mm256_add_epi32(hash4d_0110_1110, hash_mat_WW);

        __m256i _m256i_HASH_FACTOR = _mm256_set1_epi32(HASH_FACTOR);
                                                                    
        hash4d_0000_1000 = _mm256_mullo_epi32(_mm256_xor_si256(hash4d_0000_1000, _mm256_srli_epi32(hash4d_0000_1000, HASH_SHIFT)), _m256i_HASH_FACTOR);
        hash4d_0100_1100 = _mm256_mullo_epi32(_mm256_xor_si256(hash4d_0100_1100, _mm256_srli_epi32(hash4d_0100_1100, HASH_SHIFT)), _m256i_HASH_FACTOR);
        hash4d_0010_1010 = _mm256_mullo_epi32(_mm256_xor_si256(hash4d_0010_1010, _mm256_srli_epi32(hash4d_0010_1010, HASH_SHIFT)), _m256i_HASH_FACTOR);
        hash4d_0110_1110 = _mm256_mullo_epi32(_mm256_xor_si256(hash4d_0110_1110, _mm256_srli_epi32(hash4d_0110_1110, HASH_SHIFT)), _m256i_HASH_FACTOR);
        hash4d_0001_1001 = _mm256_mullo_epi32(_mm256_xor_si256(hash4d_0001_1001, _mm256_srli_epi32(hash4d_0001_1001, HASH_SHIFT)), _m256i_HASH_FACTOR);
        hash4d_0101_1101 = _mm256_mullo_epi32(_mm256_xor_si256(hash4d_0101_1101, _mm256_srli_epi32(hash4d_0101_1101, HASH_SHIFT)), _m256i_HASH_FACTOR);
        hash4d_0011_1011 = _mm256_mullo_epi32(_mm256_xor_si256(hash4d_0011_1011, _mm256_srli_epi32(hash4d_0011_1011, HASH_SHIFT)), _m256i_HASH_FACTOR);
        hash4d_0111_1111 = _mm256_mullo_epi32(_mm256_xor_si256(hash4d_0111_1111, _mm256_srli_epi32(hash4d_0111_1111, HASH_SHIFT)), _m256i_HASH_FACTOR);

        __m256 grad_0000_1000 = _mm256_cvtepi32_ps(hash4d_0000_1000);
        __m256 grad_0100_1100 = _mm256_cvtepi32_ps(hash4d_0100_1100);
        __m256 grad_0010_1010 = _mm256_cvtepi32_ps(hash4d_0010_1010);
        __m256 grad_0110_1110 = _mm256_cvtepi32_ps(hash4d_0110_1110);
        __m256 grad_0001_1001 = _mm256_cvtepi32_ps(hash4d_0001_1001);
        __m256 grad_0101_1101 = _mm256_cvtepi32_ps(hash4d_0101_1101);
        __m256 grad_0011_1011 = _mm256_cvtepi32_ps(hash4d_0011_1011);
        __m256 grad_0111_1111 = _mm256_cvtepi32_ps(hash4d_0111_1111);

        __m256 Pf0Pf0 = _mm256_insertf128_ps(_mm256_castps128_ps256(Pf0), Pf0, 1);
        __m256 Pf1Pf1 = _mm256_insertf128_ps(_mm256_castps128_ps256(Pf1), Pf1, 1);

        __m256 dp_0000_1000 = _mm256_blend_ps(Pf0Pf0, Pf1Pf1, _MM_BLEND_MASK8(0, 0, 0, 0, 1, 0, 0, 0));
        __m256 dp_0100_1100 = _mm256_blend_ps(Pf0Pf0, Pf1Pf1, _MM_BLEND_MASK8(0, 1, 0, 0, 1, 1, 0, 0));
        __m256 dp_0010_1010 = _mm256_blend_ps(Pf0Pf0, Pf1Pf1, _MM_BLEND_MASK8(0, 0, 1, 0, 1, 0, 1, 0));
        __m256 dp_0110_1110 = _mm256_blend_ps(Pf0Pf0, Pf1Pf1, _MM_BLEND_MASK8(0, 1, 1, 0, 1, 1, 1, 0));
        __m256 dp_0001_1001 = _mm256_blend_ps(Pf0Pf0, Pf1Pf1, _MM_BLEND_MASK8(0, 0, 0, 1, 1, 0, 0, 1));
        __m256 dp_0101_1101 = _mm256_blend_ps(Pf0Pf0, Pf1Pf1, _MM_BLEND_MASK8(0, 1, 0, 1, 1, 1, 0, 1));
        __m256 dp_0011_1011 = _mm256_blend_ps(Pf0Pf0, Pf1Pf1, _MM_BLEND_MASK8(0, 0, 1, 1, 1, 0, 1, 1));
        __m256 dp_0111_1111 = _mm256_blend_ps(Pf0Pf0, Pf1Pf1, _MM_BLEND_MASK8(0, 1, 1, 1, 1, 1, 1, 1));

        __m256 tmp1 = _mm256_hadd_ps(_mm256_mul_ps(grad_0000_1000, dp_0000_1000), _mm256_mul_ps(grad_0010_1010, dp_0010_1010));
        __m256 tmp2 = _mm256_hadd_ps(_mm256_mul_ps(grad_0100_1100, dp_0100_1100), _mm256_mul_ps(grad_0110_1110, dp_0110_1110));
        __m256 dp_00_10 = _mm256_hadd_ps(tmp1, tmp2);

        tmp1 = _mm256_hadd_ps(_mm256_mul_ps(grad_0001_1001, dp_0001_1001), _mm256_mul_ps(grad_0011_1011, dp_0011_1011));
        tmp2 = _mm256_hadd_ps(_mm256_mul_ps(grad_0101_1101, dp_0101_1101), _mm256_mul_ps(grad_0111_1111, dp_0111_1111));
        __m256 dp_01_11 = _mm256_hadd_ps(tmp1, tmp2);

        __m128 ws_ws_ws_ws = _mm_permute_ps(Ps, _MM_SHUFFLE(3, 3, 3, 3));
        __m256 ws_ws_ws_ws_ws_ws_ws_ws = _mm256_insertf128_ps(_mm256_castps128_ps256(ws_ws_ws_ws), ws_ws_ws_ws, 1);
        __m256 grad_w = _mm256_fmadd_ps(_mm256_sub_ps(dp_01_11, dp_00_10), ws_ws_ws_ws_ws_ws_ws_ws, dp_00_10);

        __m128 grad_z0 = _mm256_castps256_ps128(grad_w);
        __m128 grad_z1 = _mm256_extractf128_ps(grad_w, 1);        
        __m128 zs_zs_zs_zs = _mm_permute_ps(Ps, _MM_SHUFFLE(2, 2, 2, 2));
        __m128 grad_z = _mm_fmadd_ps(_mm_sub_ps(grad_z1, grad_z0), zs_zs_zs_zs, grad_z0);

        __m128 grad_y0 = grad_z;
        __m128 grad_y1 = _mm_movehl_ps(grad_z, grad_z);
        __m128 ys_ys_ys_ys = _mm_permute_ps(Ps, _MM_SHUFFLE(1, 1, 1, 1));
        __m128 grad_y = _mm_fmadd_ps(_mm_sub_ps(grad_y1, grad_y0), ys_ys_ys_ys, grad_y0);
        __m128 grad_x = _mm_fmadd_ss(_mm_sub_ps(_mm_permute_ps(grad_y, _MM_SHUFFLE(3,2,1,1)), grad_y), Ps, grad_y);

        return GRADIENT_4D_AVX2_NORMALIZATION_FACTOR_F * _mm_cvtss_f32(grad_x);
    }


    template<> double gradient_avx2(const glm::dvec4& arg)
    {
        __m256d P = _mm256_loadu_pd(glm::value_ptr(arg));                                           // P    = |  x  :  y  :  z  :  w  |
        __m256d Pid = _mm256_floor_pd(P);                                                           // Pid  = | [x] : [y] : [z] : [w] | as doubles
        __m128i Pi = _mm256_cvtpd_epi32(Pid);                                                       // Pi   = | [x] : [y] : [z] : [w] | as integers
        __m256d Pf0 = _mm256_sub_pd(P, Pid);                                                        // Pf0  = | {x} : {y} : {z} : {w} |
        __m256d Ps = smooth_step5(Pf0);                                                             // Ps0  = | s_x : s_y : s_z : s_w |

        const double one = 1.0;
        __m256d _m256d_one = _mm256_broadcast_sd(&one);
        __m256d Pf1 = _mm256_sub_pd(Pf0, _m256d_one);                                               // Pf0  = | {x} - 1 : {y} - 1 : {z} - 1 : {w} - 1 |

        __m128i hash_mat_X = _mm_load_si128((const __m128i*)glm::value_ptr(hash_mat4x4[0]));        // X - column of the hash matrix
        __m128i hash_mat_Y = _mm_load_si128((const __m128i*)glm::value_ptr(hash_mat4x4[1]));        // Y - column of the hash matrix
        __m128i hash_mat_Z = _mm_load_si128((const __m128i*)glm::value_ptr(hash_mat4x4[2]));        // Z - column of the hash matrix
        __m128i hash_mat_W = _mm_load_si128((const __m128i*)glm::value_ptr(hash_mat4x4[3]));        // W - column of the hash matrix

        __m128i hash0000 = _mm_add_epi32(
                                _mm_add_epi32(
                                    _mm_mullo_epi32(hash_mat_X, _mm_shuffle_epi32(Pi, _MM_SHUFFLE(0,0,0,0))),
                                    _mm_mullo_epi32(hash_mat_Y, _mm_shuffle_epi32(Pi, _MM_SHUFFLE(1,1,1,1)))
                                ),
                                _mm_add_epi32(
                                    _mm_mullo_epi32(hash_mat_Z, _mm_shuffle_epi32(Pi, _MM_SHUFFLE(2,2,2,2))),
                                    _mm_mullo_epi32(hash_mat_W, _mm_shuffle_epi32(Pi, _MM_SHUFFLE(3,3,3,3)))
                                )
                            );

        __m128i hash1000 = _mm_add_epi32(hash0000, hash_mat_X);

        __m256i hash4d_0000_1000 = _mm256_inserti128_si256(_mm256_castsi128_si256(hash0000), hash1000, 1);
        __m256i hash4d_0100_1100 = _mm256_add_epi32(hash4d_0000_1000, _mm256_broadcastsi128_si256(hash_mat_Y));

        __m256i hash_mat_ZZ = _mm256_broadcastsi128_si256(hash_mat_Z); 
        __m256i hash_mat_WW = _mm256_broadcastsi128_si256(hash_mat_W); 

        __m256i hash4d_0010_1010 = _mm256_add_epi32(hash4d_0000_1000, hash_mat_ZZ);
        __m256i hash4d_0110_1110 = _mm256_add_epi32(hash4d_0100_1100, hash_mat_ZZ);
        __m256i hash4d_0001_1001 = _mm256_add_epi32(hash4d_0000_1000, hash_mat_WW);
        __m256i hash4d_0101_1101 = _mm256_add_epi32(hash4d_0100_1100, hash_mat_WW);
        __m256i hash4d_0011_1011 = _mm256_add_epi32(hash4d_0010_1010, hash_mat_WW);
        __m256i hash4d_0111_1111 = _mm256_add_epi32(hash4d_0110_1110, hash_mat_WW);

        __m256i _m256i_HASH_FACTOR = _mm256_set1_epi32(HASH_FACTOR);
                                                                    
        hash4d_0000_1000 = _mm256_mullo_epi32(_mm256_xor_si256(hash4d_0000_1000, _mm256_srli_epi32(hash4d_0000_1000, HASH_SHIFT)), _m256i_HASH_FACTOR);
        hash4d_0100_1100 = _mm256_mullo_epi32(_mm256_xor_si256(hash4d_0100_1100, _mm256_srli_epi32(hash4d_0100_1100, HASH_SHIFT)), _m256i_HASH_FACTOR);
        hash4d_0010_1010 = _mm256_mullo_epi32(_mm256_xor_si256(hash4d_0010_1010, _mm256_srli_epi32(hash4d_0010_1010, HASH_SHIFT)), _m256i_HASH_FACTOR);
        hash4d_0110_1110 = _mm256_mullo_epi32(_mm256_xor_si256(hash4d_0110_1110, _mm256_srli_epi32(hash4d_0110_1110, HASH_SHIFT)), _m256i_HASH_FACTOR);
        hash4d_0001_1001 = _mm256_mullo_epi32(_mm256_xor_si256(hash4d_0001_1001, _mm256_srli_epi32(hash4d_0001_1001, HASH_SHIFT)), _m256i_HASH_FACTOR);
        hash4d_0101_1101 = _mm256_mullo_epi32(_mm256_xor_si256(hash4d_0101_1101, _mm256_srli_epi32(hash4d_0101_1101, HASH_SHIFT)), _m256i_HASH_FACTOR);
        hash4d_0011_1011 = _mm256_mullo_epi32(_mm256_xor_si256(hash4d_0011_1011, _mm256_srli_epi32(hash4d_0011_1011, HASH_SHIFT)), _m256i_HASH_FACTOR);
        hash4d_0111_1111 = _mm256_mullo_epi32(_mm256_xor_si256(hash4d_0111_1111, _mm256_srli_epi32(hash4d_0111_1111, HASH_SHIFT)), _m256i_HASH_FACTOR);

        __m256d grad0000 = _mm256_cvtepi32_pd(_mm256_castsi256_si128(hash4d_0000_1000));
        __m256d grad1000 = _mm256_cvtepi32_pd(_mm256_extracti128_si256(hash4d_0000_1000, 1));
        __m256d grad0100 = _mm256_cvtepi32_pd(_mm256_castsi256_si128(hash4d_0100_1100));
        __m256d grad1100 = _mm256_cvtepi32_pd(_mm256_extracti128_si256(hash4d_0100_1100, 1));
        __m256d grad0010 = _mm256_cvtepi32_pd(_mm256_castsi256_si128(hash4d_0010_1010));
        __m256d grad1010 = _mm256_cvtepi32_pd(_mm256_extracti128_si256(hash4d_0010_1010, 1));
        __m256d grad0110 = _mm256_cvtepi32_pd(_mm256_castsi256_si128(hash4d_0110_1110));
        __m256d grad1110 = _mm256_cvtepi32_pd(_mm256_extracti128_si256(hash4d_0110_1110, 1));
        __m256d grad0001 = _mm256_cvtepi32_pd(_mm256_castsi256_si128(hash4d_0001_1001));
        __m256d grad1001 = _mm256_cvtepi32_pd(_mm256_extracti128_si256(hash4d_0001_1001, 1));
        __m256d grad0101 = _mm256_cvtepi32_pd(_mm256_castsi256_si128(hash4d_0101_1101));
        __m256d grad1101 = _mm256_cvtepi32_pd(_mm256_extracti128_si256(hash4d_0101_1101, 1));
        __m256d grad0011 = _mm256_cvtepi32_pd(_mm256_castsi256_si128(hash4d_0011_1011));
        __m256d grad1011 = _mm256_cvtepi32_pd(_mm256_extracti128_si256(hash4d_0011_1011, 1));
        __m256d grad0111 = _mm256_cvtepi32_pd(_mm256_castsi256_si128(hash4d_0111_1111));
        __m256d grad1111 = _mm256_cvtepi32_pd(_mm256_extracti128_si256(hash4d_0111_1111, 1));


        __m256d dp0000 = _mm256_blend_pd(Pf0, Pf1, _MM_BLEND_MASK4(0, 0, 0, 0));
        __m256d dp1000 = _mm256_blend_pd(Pf0, Pf1, _MM_BLEND_MASK4(1, 0, 0, 0));
        __m256d dp0100 = _mm256_blend_pd(Pf0, Pf1, _MM_BLEND_MASK4(0, 1, 0, 0));
        __m256d dp1100 = _mm256_blend_pd(Pf0, Pf1, _MM_BLEND_MASK4(1, 1, 0, 0));
        __m256d dp0010 = _mm256_blend_pd(Pf0, Pf1, _MM_BLEND_MASK4(0, 0, 1, 0));
        __m256d dp1010 = _mm256_blend_pd(Pf0, Pf1, _MM_BLEND_MASK4(1, 0, 1, 0));
        __m256d dp0110 = _mm256_blend_pd(Pf0, Pf1, _MM_BLEND_MASK4(0, 1, 1, 0));
        __m256d dp1110 = _mm256_blend_pd(Pf0, Pf1, _MM_BLEND_MASK4(1, 1, 1, 0));
        __m256d dp0001 = _mm256_blend_pd(Pf0, Pf1, _MM_BLEND_MASK4(0, 0, 0, 1));
        __m256d dp1001 = _mm256_blend_pd(Pf0, Pf1, _MM_BLEND_MASK4(1, 0, 0, 1));
        __m256d dp0101 = _mm256_blend_pd(Pf0, Pf1, _MM_BLEND_MASK4(0, 1, 0, 1));
        __m256d dp1101 = _mm256_blend_pd(Pf0, Pf1, _MM_BLEND_MASK4(1, 1, 0, 1));
        __m256d dp0011 = _mm256_blend_pd(Pf0, Pf1, _MM_BLEND_MASK4(0, 0, 1, 1));
        __m256d dp1011 = _mm256_blend_pd(Pf0, Pf1, _MM_BLEND_MASK4(1, 0, 1, 1));
        __m256d dp0111 = _mm256_blend_pd(Pf0, Pf1, _MM_BLEND_MASK4(0, 1, 1, 1));
        __m256d dp1111 = _mm256_blend_pd(Pf0, Pf1, _MM_BLEND_MASK4(1, 1, 1, 1));
        
        __m256d tmp1 = _mm256_hadd_pd(_mm256_mul_pd(grad0000, dp0000), _mm256_mul_pd(grad0100, dp0100));
        __m256d tmp2 = _mm256_hadd_pd(_mm256_mul_pd(grad1000, dp1000), _mm256_mul_pd(grad1100, dp1100));
        tmp1 = _mm256_permute4x64_pd(tmp1, _MM_SHUFFLE(3,1,2,0));
        tmp2 = _mm256_permute4x64_pd(tmp2, _MM_SHUFFLE(3,1,2,0));
        __m256d grad_zw00 = _mm256_hadd_pd(tmp1, tmp2);

        tmp1 = _mm256_hadd_pd(_mm256_mul_pd(grad0010, dp0010), _mm256_mul_pd(grad0110, dp0110));
        tmp2 = _mm256_hadd_pd(_mm256_mul_pd(grad1010, dp1010), _mm256_mul_pd(grad1110, dp1110));
        tmp1 = _mm256_permute4x64_pd(tmp1, _MM_SHUFFLE(3,1,2,0));
        tmp2 = _mm256_permute4x64_pd(tmp2, _MM_SHUFFLE(3,1,2,0));
        __m256d grad_zw10 = _mm256_hadd_pd(tmp1, tmp2);

        tmp1 = _mm256_hadd_pd(_mm256_mul_pd(grad0001, dp0001), _mm256_mul_pd(grad0101, dp0101));
        tmp2 = _mm256_hadd_pd(_mm256_mul_pd(grad1001, dp1001), _mm256_mul_pd(grad1101, dp1101));
        tmp1 = _mm256_permute4x64_pd(tmp1, _MM_SHUFFLE(3,1,2,0));
        tmp2 = _mm256_permute4x64_pd(tmp2, _MM_SHUFFLE(3,1,2,0));
        __m256d grad_zw01 = _mm256_hadd_pd(tmp1, tmp2);

        tmp1 = _mm256_hadd_pd(_mm256_mul_pd(grad0011, dp0011), _mm256_mul_pd(grad0111, dp0111));
        tmp2 = _mm256_hadd_pd(_mm256_mul_pd(grad1011, dp1011), _mm256_mul_pd(grad1111, dp1111));
        tmp1 = _mm256_permute4x64_pd(tmp1, _MM_SHUFFLE(3,1,2,0));
        tmp2 = _mm256_permute4x64_pd(tmp2, _MM_SHUFFLE(3,1,2,0));

        __m256d grad_zw11 = _mm256_hadd_pd(tmp1, tmp2);
        __m256d ws_ws_ws_ws = _mm256_permute4x64_pd(Ps, _MM_SHUFFLE(3, 3, 3, 3));
        __m256d zs_zs_zs_zs = _mm256_permute4x64_pd(Ps, _MM_SHUFFLE(2, 2, 2, 2));

        __m256d grad_z0 = _mm256_fmadd_pd(_mm256_sub_pd(grad_zw01, grad_zw00), ws_ws_ws_ws, grad_zw00);
        __m256d grad_z1 = _mm256_fmadd_pd(_mm256_sub_pd(grad_zw11, grad_zw10), ws_ws_ws_ws, grad_zw10);

        __m256d grad_zw = _mm256_fmadd_pd(_mm256_sub_pd(grad_z1, grad_z0), zs_zs_zs_zs, grad_z0);

        __m128d xs_ys = _mm256_castpd256_pd128(Ps);
        __m128d ys_ys = _mm_unpackhi_pd(xs_ys, xs_ys);

        __m128d grad_y0 = _mm256_castpd256_pd128(grad_zw);
        __m128d grad_y1 = _mm256_extractf128_pd(grad_zw, 1);

        __m128d grad_y = _mm_fmadd_pd(_mm_sub_pd(grad_y1, grad_y0), ys_ys, grad_y0);

        __m128d grad = _mm_fmadd_sd(_mm_sub_sd(_mm_unpackhi_pd(grad_y, grad_y), grad_y), xs_ys, grad_y);

                  
        return GRADIENT_4D_AVX2_NORMALIZATION_FACTOR * _mm_cvtsd_f64(grad);
    }

} // namespace noise
