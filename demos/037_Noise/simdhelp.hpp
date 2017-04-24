#ifndef _simd_help_included_356138749651027456137561084756983476598347651893456
#define _simd_help_included_356138749651027456137561084756983476598347651893456

#include <cstdio>

//#define ENABLE_PRINT

#if defined(__SSE2__)
    inline void print_register(const char* desc, const __m128& r)
    {
        #ifdef ENABLE_PRINT
            printf("%s [%.5f : %.5f : %.5f : %.5f]\n", desc, r[0], r[1], r[2], r[3]);
        #endif
    }

    inline void print_register(const char* desc, const __m128d& r)
    {
        #ifdef ENABLE_PRINT
            printf("%s [%.5f : %.5f]\n", desc, r[0], r[1]);
        #endif
    }

    inline void print_register(const char* desc, const __m128i& r)
    {
        #ifdef ENABLE_PRINT
            printf("%s [%d : %d : %d : %d]\n", desc, int32_t(r[0]), int32_t(r[0] >> 32), int32_t(r[1]), int32_t(r[1] >> 32));
        #endif
    }
#endif

#if defined(__AVX2__)
    inline void print_register(const char* desc, const __m256& r)
    {
        #ifdef ENABLE_PRINT
            printf("%s [%.5f : %.5f : %.5f : %.5f : %.5f : %.5f : %.5f : %.5f]\n", desc, r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7]);
        #endif
    }

    inline void print_register(const char* desc, const __m256d& r)
    {
        #ifdef ENABLE_PRINT
            printf("%s [%.5f : %.5f : %.5f : %.5f]\n", desc, r[0], r[1], r[2], r[3]);
        #endif
    }

    inline void print_register(const char* desc, const __m256i& r)
    {
        #ifdef ENABLE_PRINT
            printf("%s [%d : %d : %d : %d : %d : %d : %d : %d]\n", desc, int32_t(r[0]), int32_t(r[0] >> 32), int32_t(r[1]), int32_t(r[1] >> 32), int32_t(r[2]), int32_t(r[2] >> 32), int32_t(r[3]), int32_t(r[3] >> 32));
        #endif
    }
#endif

    //===================================================================================================================================================================================================================
    //
    // SSE 2 Useful stuff :
    //
    // 1. Undefined register generators (to be used for mask generations and single/half-register/horizontal calculations): 
    //      __m128  _mm_undefined_ps(void);
    //      __m128d _mm_undefined_pd(void);
    //      __m128i _mm_undefined_si128(void);
    //      __m256  _mm256_undefined_ps(void);
    //      __m256d _mm256_undefined_pd(void);
    //      __m256i _mm256_undefined_si256(void);
    //
    //===================================================================================================================================================================================================================

    //===================================================================================================================================================================================================================
    //
    // SSE 3 :
    //
    // 1. ADDSUBPS :
    //      _mm_addsub_ps(__m128 a, __m128 b) = | a0 - b0 : a1 + b1 : a2 - b2 : a3 + b3 |
    //
    // 2. HADDPS : 
    //      _mm_hadd_ps(__m128 a, __m128 b) = | a0 + a1 : a2 + a3 : b0 + b1 : b2 + b3 |
    //
    // 3. HSUBPS
    //      _mm_hsub_ps(__m128 a, __m128 b) = | a0 - a1 : a2 - a3 : b0 - b1 : b2 - b3 |
    //
    // 4. MOVSLDUP/MOVSHDUP
    //      _mm_moveldup_ps(__m128 a) = | a0 : a0 : a2 : a2 |
    //      _mm_movehdup_ps(__m128 a) = | a1 : a1 : a3 : a3 |
    // 
    //===================================================================================================================================================================================================================

    //===================================================================================================================================================================================================================
    //
    // SSE 4.1 Additional useful instructions:
    //
    // 1. HADDPD/HADDPS :: Horizontal addition of double/single precision floating-point values.
    //      intrinsics : _mm_hadd_pd, _mm_hadd_ps 
    //
    // 2. ROUNDPD/ROUNDPS :: Round packed double/single precision floating-point values. The rounding mode is determined by immediate byte argument.
    //      intrinsics : _mm_round_pd, _mm_round_ps, _mm_floor_pd, _mm_floor_ps 
    //
    // 3. PMULLD :: Multiply packed signed dword integers and store low result
    //      intrinsics : _mm_mullo_epi32
    //
    // 4. DPPS/DPPD :: Dot product of packed single precision floating-point values
    //      intrinsics : _mm_dp_pd/_mm_dp_ps
    //
    //===================================================================================================================================================================================================================

    //===================================================================================================================================================================================================================
    //
    // AVX 2:
    //
    // 1. VBROADCASTSD:
    //      _mm_broadcastsd_pd(__m128d a) = | a0 : a0 |
    //      _mm256_broadcastsd_pd(__m128d a) = | a0 : a0 : a0 : a0 |
    //      _mm_broadcastss_ps(__m128 a)  = | a0 : a0 : a0 : a0 |
    //      _mm256_broadcastss_ps(__m128 val)  = | a0 : a0 : a0 : a0 : a0 : a0 : a0 : a0 |
    //
    // 2. VFMADD132PD, VFMADD213PD, VFMADD231PD (numbers determine destination register out of 3 operands):
    //      _mm256_fmadd_pd(__m256d a, __m256d b, __m256d c) = | a0 * b0 + c0 : a1 * b1 + c1 : a2 * b2 + c2 : a3 * b3 + c3 |
    //      _mm256_fmsubadd_pd(__m256d a, __m256d b, __m256d c) = | a0 * b0 - c0 : a1 * b1 + c1 : a2 * b2 - c2 : a3 * b3 + c3 |
    //      _mm256_fnmadd_ps(__m256 a, __m256 b, __m256 c) = | c0 - a0 * b0 : c1 - a1 * b1 : c2 - a2 * b2 : c3 - a3 * b3 |
    //      _mm256_fnmsub_pd(__m256d a, __m256d b, __m256d c) = | -a0 * b0 - c0 : -a1 * b1 - c1 : -a2 * b2 - c2 : -a3 * b3 - c3 |
    //
    // 3. VINSERTF128
    //      _mm256_insertf128_pd(__m256d a, __m128d b, const int mask) = (mask == 0) ? | b : a1_128 | : | a0_128 : b |
    //      _mm256_inserti128_si256(__m256i a, __m128i b, const int mask) = (mask == 0) ? | b : a1_128 | : | a0_128 : b |
    // 
    // 4. VDPPS, single precision : 
    //      dp0 = (mask & 16) * a0 * b0 + (mask & 32) * a1 * b1 + (mask & 64) * a2 * b2 + (mask & 128) * a3 * b3  
    //      dp1 = (mask & 16) * a4 * b4 + (mask & 32) * a5 * b5 + (mask & 64) * a6 * b6 + (mask & 128) * a7 * b7
    //      _mm256_dp_ps ( __m256 a, __m256 b, const int mask) = 
    //          | (mask & 1) ? dp0 : (mask & 2) ? dp0 : (mask & 4) ? dp0 : (mask & 8) ? dp0 : 
    //            (mask & 1) ? dp1 : (mask & 2) ? dp1 : (mask & 4) ? dp1 : (mask & 8) ? dp1 |   
    //      _mm_dp_ps (__m128 a, __m128 b, const int mask) = | (mask & 1) ? dp0 : (mask & 2) ? dp0 : (mask & 4) ? dp0 : (mask & 8) ? dp0 |
    //
    //    VDPPD, double precision :
    //      dp0 = (mask & 16) * a0 * b0 + (mask & 32) * a1 * b1;
    //      _mm_dp_pd(__m128d a, __m128d b, const int mask) = | (mask & 1) ? dp0 : (mask & 2) ? dp0 |
    //
    // 5. VUNPCKHPD / VUNPCKLPD / VUNPCKHPS / VUNPCKLPS
    //      _mm256_unpackhi_pd(__m256d a, __m256d b) = | a1 : b1 : a3 : b3 |
    //      _mm256_unpacklo_pd(__m256d a, __m256d b) = | a0 : b0 : a2 : b2 |
    //      _mm256_unpackhi_ps(__m256 m1, __m256 m2) = | a1 : b1 : a3 : b3 : a5 : b5 : a7 : b7 |
    //      _mm256_unpacklo_ps(__m256 m1, __m256 m2) = | a0 : b0 : a2 : b2 : a4 : b4 : a6 : b6 |
    //
    //===================================================================================================================================================================================================================

    //===================================================================================================================================================================================================================
    //  SIMD branching-free code tricks :: 
    //
    //  1. Getting single precision floating point 1.0f (0x3F800000) from mask = 0xFFFFFFFF 
    // can be done either by (mask >> 25) << 23 or by (mask << 25) >> 2.
    //
    //  2. Getting double precision floating point 1.0f (0x3FF0000000000000) from mask = 0xFFFFFFFFFFFFFFFF 
    // either (mask >> 54) << 52 or (mask << 54) >> 2 can be used.
    //
    //
    // inline __m128 _mm_floor_ps2(const __m128& x)
    // {
    //     __m128i ix = _mm_cvttps_epi32(x);                                                       //    ix = | int(x_0) : int(x_1) : int(x_2) : int(x_3) | --- int32s
    //     __m128 ixf = _mm_cvtepi32_ps(ix);                                                       //   ixf = | int(x_0) : int(x_1) : int(x_2) : int(x_3) | --- floats
    //     __m128i maski = _mm_castps_si128(_mm_cmpgt_ps(ixf, x));                                 // maski = (ixf > x) ? 0xFFFFFFFF : 0
    //     __m128 maskf = _mm_castsi128_ps(_mm_slli_epi32(_mm_srli_epi32(maski, 25), 23));         // maskf = (ixf > x) ? 0x3F800000 : 0 = 1.0f : 0.0f
    //   //__m128 maskf = _mm_castsi128_ps(_mm_srli_epi32(_mm_slli_epi32(maski, 25), 2));          // maskf = (ixf > x) ? 0x3F800000 : 0 = 1.0f : 0.0f
    //     return _mm_sub_ps(ixf, maskf);
    // }
    //
    //===================================================================================================================================================================================================================
    //
    // inline __m128d _mm_floor_ps2(const __m128d& x)
    // {
    //     __m128i ix = _mm_cvttpd_epi32(x);                                                       //    ix = | int(x_0) : int(x_1) : 0 : 0 | --- int32s
    //     __m128d ixf = _mm_cvtepi32_pd(ix);                                                      //   ixf = | int(x_0) : int(x_1) : 0 : 0 | --- floats
    //     __m128i maski = _mm_castpd_si128(_mm_cmpgt_pd(ixf, x));                                 // maski = (ixf > x) ? 0xFFFFFFFFFFFFFFFF : 0
    //     __m128d maskd = _mm_castsi128_pd(_mm_slli_epi64(_mm_srli_epi64(maski, 54), 52));        // maskf = (ixf > x) ? 0x3FF0000000000000 : 0 = 1.0 : 0.0
    //   //__m128d maskd = _mm_castsi128_pd(_mm_srli_epi64(_mm_slli_epi64(maski, 54), 2));         // maskf = (ixf > x) ? 0x3FF0000000000000 : 0 = 1.0 : 0.0
    //     return _mm_sub_pd(ixf, maskd);
    // }
    //
    //===================================================================================================================================================================================================================

#endif // _simd_help_included_356138749651027456137561084756983476598347651893456