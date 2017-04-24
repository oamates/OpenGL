#include <glm/gtc/type_ptr.hpp>

#include "sse2.hpp"
#include "common.hpp"

namespace noise {

//=======================================================================================================================================================================================================================
// SSE2 implementation of value noise                                                                                                                                                                                   
//=======================================================================================================================================================================================================================
    template<> float value_sse2<float>(const glm::vec2& P) 
    {
        return 0.0f;
    }

    template<> double value_sse2<double>(const glm::dvec2& arg) 
    {
        __m128d P = _mm_load_pd(glm::value_ptr(arg));
        __m128i Pi = _mm_cvtpd_epi32(P);                                                            // Pi  = | (x) : (y) : 0 : 0 | as integers
        __m128d Pid = _mm_cvtepi32_pd(Pi);                                                          // Pid = | (x) : (y) | as doubles
        __m128d Pf = _mm_sub_pd(P, Pid);                                                            // Pf  = | <x> : <y> | as doubles
        __m128i maski = _mm_castpd_si128(_mm_cmplt_pd(P, Pid));                                     // maski = (P < Pid) ? 0xFFFFFFFFFFFFFFFF : 0x0000000000000000
        __m128d maskd = _mm_castsi128_pd(_mm_slli_epi64(_mm_srli_epi64(maski, 54), 52));            // maskd = (P < Pid) ? 0x3FF0000000000000 : 0 = 1.0 : 0.0

        Pf = _mm_add_pd(Pf, maskd);                                                                 // add correction to the fractional part
        __m128d Ps = smooth_step5(Pf);                                                              // Ps  = | s_x : s_y |
        Pi = _mm_unpacklo_epi32(Pi, Pi);                                                            // Pi = | int(x) : int(x) : int(y) : int(y) |
        Pi = _mm_add_epi32(Pi, maski);                                                              // Pi = | [x] : [x] : [y] : [y] |

        __m128i Pm = _mm_mul_epu32(Pi, _m128i_HASH_FACTOR_X0Y0);                                    // Pm = | [x] * F_X : [y] * F_Y | as int64
        Pm = _mm_shuffle_epi32(Pm, _MM_SHUFFLE(2, 2, 0, 0));                                        // Pm = | [x] * F_X : [x] * F_X : [y] * F_Y : [y] * F_Y | lower 32-bit parts
        Pm = _mm_add_epi32(Pm, _m128i_HASH_FACTOR_X0Y0);                                            // Pm = | ([x] + 1) * F_X : [x] * F_X : ([y] + 1) * F_Y : [y] * F_Y |

        __m128i hash = _mm_add_epi32(
                            _mm_shuffle_epi32(Pm, _MM_SHUFFLE(0, 0, 1, 1)),                         // | [x] * F_X : [x] * F_X : ([x] + 1) * F_X : ([x] + 1) * F_X | 
                            _mm_shuffle_epi32(Pm, _MM_SHUFFLE(2, 3, 2, 3))                          // | [y] * F_Y : ([y] + 1) * F_Y : [y] * F_Y : ([y] + 1) * F_Y |
                        );                                                                          // hash = | hash(0, 0) : hash(0, 1) : hash(1, 0) : hash(1, 1) |
                                                                                                    
        hash = _mm_xor_si128(hash, _mm_srli_epi32(hash, HASH_SHIFT));
        __m128i hash_factor = _mm_set1_epi32(HASH_FACTOR);
        __m128i hash_00_10 = _mm_mul_epu32(hash, hash_factor);
        __m128i hash_01_11 = _mm_mul_epu32(_mm_srli_si128(hash, 4), hash_factor);

        __m128d val_00_10 = _mm_cvtepi32_pd(_mm_shuffle_epi32(hash_00_10, _MM_SHUFFLE(3,1,2,0)));
        __m128d val_01_11 = _mm_cvtepi32_pd(_mm_shuffle_epi32(hash_01_11, _MM_SHUFFLE(3,1,2,0)));

        __m128d val_x01 = _mm_add_pd(val_00_10, _mm_mul_pd(_mm_sub_pd(val_01_11, val_00_10), _mm_unpackhi_pd(Ps, Ps)));
        __m128d val = _mm_add_sd(val_x01, _mm_mul_sd(_mm_sub_sd(_mm_unpackhi_pd(val_x01, val_x01), val_x01), Ps));
        return _mm_cvtsd_f64(val) * VALUE_2D_SSE2_NORMALIZATION_FACTOR;
    }

template<> float value_sse2(const glm::vec3& P)
{
    return 0.0f;
}

template<> double value_sse2(const glm::dvec3& P)
{
    return 0.0;
}

    template<> float value_sse2(const glm::vec4& arg)
    {
        __m128 P = _mm_loadu_ps(glm::value_ptr(arg));                                               // P = | x : y : z : 0 |

        __m128i Pi = _mm_cvttps_epi32(P);                                                           // Pi   = | int(x) : int(y) : int(z) : int(w) | as integers
        __m128 Pif = _mm_cvtepi32_ps(Pi);                                                           // Pif  = | int(x) : int(y) : int(z) : int(w) | as floats
        __m128 Pf = _mm_sub_ps(P, Pif);                                                             // Pf   = | (x) : (y) : (z) : (w) | as floats
        __m128 mask = _mm_cmplt_ps(P, Pif);                                                         // mask = (P < Pi) ? 0xFFFFFFFF : 0x00000000
        Pi = _mm_sub_epi32(Pi, _mm_srli_epi32(_mm_castps_si128(mask), 31));                         // adjust integral part Pi by subtracting 1 where this happens ...
        Pf = _mm_add_ps(Pf, _mm_and_ps(mask, _m128_1));                                             // ... and adjust floating part by adding 1.0 there 

        __m128i tmp1 = _mm_mul_epu32(Pi, _m128i_LINEAR_FACTOR_XYZW);
        __m128i tmp2 = _mm_mul_epu32(_mm_srli_si128(Pi, 4), _mm_srli_si128(_m128i_LINEAR_FACTOR_XYZW, 4));
        __m128i hash = _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1, _MM_SHUFFLE (0, 0, 2, 0)), _mm_shuffle_epi32(tmp2, _MM_SHUFFLE (0, 0, 2, 0)));

        hash = _mm_add_epi32(hash, _mm_shuffle_epi32(hash, _MM_SHUFFLE(2, 3, 0, 1)));               // linear form :: HASH_FACTOR_X * ix + HASH_FACTOR_Y * iy + HASH_FACTOR_Z * iz + HASH_FACTOR_W * iw
        hash = _mm_add_epi32(hash, _mm_shuffle_epi32(hash, _MM_SHUFFLE(0, 1, 2, 3)));               // is computed in all the components

        __m128 Ps0 = smooth_step5(Pf);                                                              // compute smoothing factors
        __m128 Ps1 = _mm_sub_ps(_m128_1, Ps0);                                                      // complimentary factors

        __m128i hash_X0 = _mm_add_epi32(hash, _M128_HASH_SHIFT_X0);                                 // pre-hashes of 4 integral points in the plane x = 0
        __m128i hash_X1 = _mm_add_epi32(hash, _M128_HASH_SHIFT_X1);                                 // pre-hashes of 4 integral points in the plane x = 1

        hash_X0 = _mm_xor_si128(_mm_srli_epi32(hash_X0, HASH_SHIFT), hash_X0);                      // hashes in the plane x = 0   ---   integers
        hash_X1 = _mm_xor_si128(_mm_srli_epi32(hash_X1, HASH_SHIFT), hash_X1);                      // hashes in the plane x = 1

        __m128 values_X0 = _mm_cvtepi32_ps(hash_X0);                                                // hashes converted to floating point values
        __m128 values_X1 = _mm_cvtepi32_ps(hash_X1);                                                // 

        __m128 xs0 = _mm_shuffle_ps(Ps1, Ps1, _MM_SHUFFLE(0, 0, 0, 0));                             // 1.0 - xs : 1.0 - xs : 1.0 - xs : 1.0 - xs
        __m128 xs1 = _mm_shuffle_ps(Ps0, Ps0, _MM_SHUFFLE(0, 0, 0, 0));                             //    xs    :    xs    :    xs    :    xs 

        __m128 tmp = _mm_shuffle_ps(Ps1, Ps0, _MM_SHUFFLE(1, 1, 1, 1));                             // 1.0 - ys : 1.0 - ys :    ys    :    ys
        __m128 ys0 = _mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(2, 0, 2, 0));                             // 1.0 - ys :    ys    : 1.0 - ys :    ys   
        __m128 ys1 = _mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(0, 2, 0, 2));                             //    ys    : 1.0 - ys :    ys    : 1.0 - ys

        __m128 zs0 = _mm_shuffle_ps(Ps1, Ps0, _MM_SHUFFLE(1, 1, 1, 1));                             // 1.0 - zs : 1.0 - zs :    zs    :    zs   
        __m128 zs1 = _mm_shuffle_ps(Ps0, Ps1, _MM_SHUFFLE(0, 0, 0, 0));                             //    zs    :    zs    : 1.0 - zs : 1.0 - zs

        __m128 value = _mm_add_ps(_mm_mul_ps(_mm_mul_ps(_mm_mul_ps(values_X0, xs0), ys0), zs0),
                                  _mm_mul_ps(_mm_mul_ps(_mm_mul_ps(values_X1, xs1), ys1), zs1));

        __m128 sum = _mm_add_ps(_mm_movehl_ps(value, value), value);
        sum = _mm_add_ss(sum, _mm_shuffle_ps(sum, sum, 1));        
        return _mm_cvtss_f32(sum);
    }

    template<> double value_sse2(const glm::dvec4& arg)
    {
        __m128d Pxy = _mm_load_pd(&arg.x);                                                          // Pxy   = |  x  :  y  |
        __m128d Pzw = _mm_load_pd(&arg.z);                                                          // Pzw   = |  z  :  w  |
        __m128i Pxyi = _mm_cvtpd_epi32(Pxy);                                                        // Pxyi  = | (x) : (y) : 0 : 0 | as integers
        __m128i Pzwi = _mm_cvtpd_epi32(Pzw);                                                        // Pzwi  = | (z) : (w) : 0 : 0 | as integers

        __m128d Pxyid = _mm_cvtepi32_pd(Pxyi);                                                      // Pxyid = | (x) : (y) | as doubles
        __m128d Pzwid = _mm_cvtepi32_pd(Pzwi);                                                      // Pzwid = | (z) : (w) | as doubles

        __m128d Pxyf = _mm_sub_pd(Pxy, Pxyid);                                                      // Pxyf  = | <x> : <y> | as doubles
        __m128d Pzwf = _mm_sub_pd(Pzw, Pzwid);                                                      // Pzwf  = | <z> : <w> | <x> = x - (x)

        __m128i maski_xy = _mm_castpd_si128(_mm_cmplt_pd(Pxy, Pxyid));                              // maski_xy = (Pxy < Pxyid) ? 0xFFFFFFFFFFFFFFFF : 0x0000000000000000
        __m128i maski_zw = _mm_castpd_si128(_mm_cmplt_pd(Pzw, Pzwid));                              // maski_zw = (Pzw < Pzwid) ? 0xFFFFFFFFFFFFFFFF : 0x0000000000000000

        __m128i Pi = _mm_unpacklo_epi64(Pxyi, Pzwi);                                                // Pi = | (x) : (y) : (z) : (w) | as integers
        __m128i mask = _mm_unpacklo_epi64(_mm_srli_si128(maski_xy, 4), _mm_srli_si128(maski_zw, 4));
        Pi = _mm_add_epi32(Pi, mask);                                                               // Pi = | [x] : [y] : [z] : [w] |

        __m128d mask_xy_d = _mm_castsi128_pd(_mm_slli_epi64(_mm_srli_epi64(maski_xy, 54), 52));     // mask_xy_d = (P < Pid) ? 0x3FF0000000000000 : 0 = 1.0 : 0.0
        __m128d mask_zw_d = _mm_castsi128_pd(_mm_slli_epi64(_mm_srli_epi64(maski_zw, 54), 52));     // mask_zw_d = (P < Pid) ? 0x3FF0000000000000 : 0 = 1.0 : 0.0

        Pxyf = _mm_add_pd(Pxyf, mask_xy_d);                                                         // Pxyf = | {x} : {y} |
        Pzwf = _mm_add_pd(Pzwf, mask_zw_d);                                                         // Pzwf = | {z} : {w} |
/*

        __m128i Pm = _mm_mul_epu32(Pi, _m128i_HASH_FACTOR_X0Y0);                                    // Pm = | [x] * F_X : [y] * F_Y | as int64
        Pm = _mm_shuffle_epi32(Pm, _MM_SHUFFLE(2, 2, 0, 0));                                        // Pm = | [x] * F_X : [x] * F_X : [y] * F_Y : [y] * F_Y | lower 32-bit parts
        Pm = _mm_add_epi32(Pm, _m128i_HASH_FACTOR_X0Y0);                                            // Pm = | ([x] + 1) * F_X : [x] * F_X : ([y] + 1) * F_Y : [y] * F_Y |

        __m128i Ph_x0011 = _mm_shuffle_epi32(Pm, _MM_SHUFFLE(0, 0, 1, 1));                          // Ph_x0011 = | [x] * F_X : [x] * F_X : ([x] + 1) * F_X : ([x] + 1) * F_X | 
        __m128i Ph_y0101 = _mm_shuffle_epi32(Pm, _MM_SHUFFLE(2, 3, 2, 3));                          // Ph_y0101 = | [y] * F_Y : ([y] + 1) * F_Y : [y] * F_Y : ([y] + 1) * F_Y | 
        __m128i hash = _mm_add_epi32(Ph_x0011, Ph_y0101);                                           // hash = | hash(0, 0) : hash(0, 1) : hash(1, 0) : hash(1, 1) |

        hash = _mm_or_si128(hash, _mm_srli_epi32(hash, HASH_SHIFT));
        __m128i tmp1 = _mm_mul_epu32(hash, _m128i_HASH_FACTOR);
        __m128i tmp2 = _mm_mul_epu32(_mm_srli_si128(hash, 4), _m128i_HASH_FACTOR);
        hash = _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1, _MM_SHUFFLE (0, 0, 2, 0)), _mm_shuffle_epi32(tmp2, _MM_SHUFFLE (0, 0, 2, 0)));

        __m128d val_00_01 = _mm_cvtepi32_pd(hash);
        __m128d val_10_11 = _mm_cvtepi32_pd(_mm_shuffle_epi32(hash, _MM_SHUFFLE(1, 0, 3, 2)));

        __m128d Ps0 = smooth_step5(Pf);
        __m128d Ps1 = _mm_sub_pd(one, Ps0);

        __m128d xs_xs = _mm_unpacklo_pd(Ps0, Ps0);
        __m128d _1mys_ys = _mm_unpackhi_pd(Ps1, Ps0);

        __m128d valY01 = _mm_add_pd(val_00_01, _mm_mul_pd(_mm_sub_pd(val_10_11, val_00_01), xs_xs));
        valY01 = _mm_mul_pd(valY01, _1mys_ys);
        valY01 = _mm_add_sd(valY01, _mm_unpackhi_pd(valY01, valY01));
        return _mm_cvtsd_f64(valY01) * VALUE_2D_SSE2_NORMALIZATION_FACTOR;
        */
        return 0.0;
    }
                                                                                                                                                                                                                         
//=======================================================================================================================================================================================================================
// SSE2 implementation of simplex noise                                                                                                                                                                                 
//=======================================================================================================================================================================================================================
template<> float simplex_sse2<float>(const glm::vec2& P) 
{
    return 0.0f;
}

template<> double simplex_sse2<double>(const glm::dvec2& P) 
{
    return 0.0;
}

template<> float simplex_sse2(const glm::vec3& P)
{
    return 0.0f;
}

template<> double simplex_sse2(const glm::dvec3& P)
{
    return 0.0;
}

template<> float simplex_sse2(const glm::vec4& P)
{
    return 0.0f;
}

template<> double simplex_sse2(const glm::dvec4& P)
{
    return 0.0;
}
                                                                                                                                                                                                                         
//=======================================================================================================================================================================================================================
// SSE2 implementation of gradient noise                                                                                                                                                                                
//=======================================================================================================================================================================================================================
template<> float gradient_sse2<float>(const glm::vec2& P) 
{
    return 0.0f;
}

    template<> double gradient_sse2<double>(const glm::dvec2& arg) 
    {
        __m128d P = _mm_load_pd(glm::value_ptr(arg));
        __m128i Pi = _mm_cvtpd_epi32(P);                                                            // Pi  = | (x) : (y) : 0 : 0 | as integers
        __m128d Pid = _mm_cvtepi32_pd(Pi);                                                          // Pid = | (x) : (y) | as doubles
        __m128d Pf00 = _mm_sub_pd(P, Pid);                                                          // Pf  = | <x> : <y> | as doubles
        __m128i maski = _mm_castpd_si128(_mm_cmplt_pd(P, Pid));                                     // maski = (P < Pid) ? 0xFFFFFFFFFFFFFFFF : 0x0000000000000000
        __m128d maskd = _mm_castsi128_pd(_mm_slli_epi64(_mm_srli_epi64(maski, 54), 52));            // maskd = (P < Pid) ? 0x3FF0000000000000 : 0 = 1.0 : 0.0

        Pf00 = _mm_add_pd(Pf00, maskd);                                                             // add correction to the fractional part
        __m128d Ps = smooth_step5(Pf00);                                                            // Ps  = | s_x : s_y |

        Pi = _mm_unpacklo_epi32(Pi, Pi);                                                            // Pi = | int(x) : int(x) : int(y) : int(y) |
        Pi = _mm_add_epi32(Pi, maski);                                                              // Pi = | [x] : [x] : [y] : [y] |

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

        __m128d dot_x01 = _mm_add_pd(dot00_10, _mm_mul_pd(_mm_sub_pd(dot01_11, dot00_10), _mm_unpackhi_pd(Ps, Ps)));
        __m128d val = _mm_add_sd(dot_x01, _mm_mul_sd(_mm_sub_sd(_mm_unpackhi_pd(dot_x01, dot_x01), dot_x01), Ps));
        return _mm_cvtsd_f64(val) * GRADIENT_2D_SSE41_NORMALIZATION_FACTOR;
    }

template<> float gradient_sse2(const glm::vec3& P)
{
    return 0.0f;
}

template<> double gradient_sse2(const glm::dvec3& P)
{
    return 0.0;
}

template<> float gradient_sse2(const glm::vec4& P)
{
    return 0.0f;
}

template<> double gradient_sse2(const glm::dvec4& P)
{
    return 0.0;
}

} // namespace noise
