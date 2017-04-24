#ifndef _noise_common_included_345627435978435627843659784356824375678239465237
#define _noise_common_included_345627435978435627843659784356824375678239465237

#define GLM_FORCE_INLINE
#define GLM_FORCE_NO_CTOR_INIT

#include <cstdint>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_integer.hpp>

namespace noise {

    //===================================================================================================================================================================================================================
    // hash constants for generating a single hash value on a lattice -- value noise
    //===================================================================================================================================================================================================================
    const uint32_t HASH_SEED = 235407543;
    const uint32_t HASH_FACTOR_X = 3367900313;
    const uint32_t HASH_FACTOR_Y = 2855995211;
    const uint32_t HASH_FACTOR_Z = 3267000013;
    const uint32_t HASH_FACTOR_W = 2860486313;
    const uint32_t HASH_SHIFT = 16;
    const uint32_t HASH_FACTOR = 3292931269;

    //===================================================================================================================================================================================================================
    // Important :: determinant of hash_mat2x2, hash_mat3x3, hash_mat4x4 matrices must be odd
    // then the linear transformation of the ring (Z/2^32Z)^k, k = 2,3,4
    // is invertible and hence is a permutation.
    // 
    // A hash value on a 2d-, 3d- or 4d-lattice point is obtained by
    //  1. taking values of 2, 3 or 4 linear forms as an initial pre-hash value
    //  2. XOR with right shift is used to randomize its lower bits
    //  3. multiplication is used to randomize higher bits.
    //
    // Note : using signed integers instead of unsigned seems making hashing even better ---
    //  high order 1's appearing after (arithmetic) right shift randomize things further.
    //===================================================================================================================================================================================================================
//#define DEBUG_MATRIX

#ifndef DEBUG_MATRIX
    const int32_t HASH_FACTOR_X0 = 3367900313;
    const int32_t HASH_FACTOR_X1 = 1241245124;
    const int32_t HASH_FACTOR_X2 = 3462347236;
    const int32_t HASH_FACTOR_X3 = 2436286764;

    const int32_t HASH_FACTOR_Y0 = 1278416582;
    const int32_t HASH_FACTOR_Y1 = 2855995211;
    const int32_t HASH_FACTOR_Y2 = 3812451242;
    const int32_t HASH_FACTOR_Y3 = 2542363476;

    const int32_t HASH_FACTOR_Z0 = 2131254344;
    const int32_t HASH_FACTOR_Z1 = 1246571626;
    const int32_t HASH_FACTOR_Z2 = 3267000013;
    const int32_t HASH_FACTOR_Z3 = 4231231946;

    const int32_t HASH_FACTOR_W0 = 4185745392;
    const int32_t HASH_FACTOR_W1 = 2517641252;
    const int32_t HASH_FACTOR_W2 = 2143352144;
    const int32_t HASH_FACTOR_W3 = 2860486313;
#else
    const int32_t HASH_FACTOR_X0 = 7;
    const int32_t HASH_FACTOR_X1 = 2;
    const int32_t HASH_FACTOR_X2 = 4;
    const int32_t HASH_FACTOR_X3 = 2;

    const int32_t HASH_FACTOR_Y0 = 4;
    const int32_t HASH_FACTOR_Y1 = 11;
    const int32_t HASH_FACTOR_Y2 = 2;
    const int32_t HASH_FACTOR_Y3 = 2;

    const int32_t HASH_FACTOR_Z0 = 2;
    const int32_t HASH_FACTOR_Z1 = 2;
    const int32_t HASH_FACTOR_Z2 = 7;
    const int32_t HASH_FACTOR_Z3 = 2;

    const int32_t HASH_FACTOR_W0 = 4;
    const int32_t HASH_FACTOR_W1 = 2;
    const int32_t HASH_FACTOR_W2 = 2;
    const int32_t HASH_FACTOR_W3 = 5;
#endif

    const glm::ivec2 hashX_vec2  = glm::ivec2(HASH_FACTOR_X0, HASH_FACTOR_X1);
    const glm::ivec2 hashY_vec2  = glm::ivec2(HASH_FACTOR_Y0, HASH_FACTOR_Y1);
    const glm::imat2 hash_mat2x2 = glm::imat2(hashX_vec2, hashY_vec2);

    const glm::ivec3 hashX_vec3  = glm::ivec3(HASH_FACTOR_X0, HASH_FACTOR_X1, HASH_FACTOR_X2);
    const glm::ivec3 hashY_vec3  = glm::ivec3(HASH_FACTOR_Y0, HASH_FACTOR_Y1, HASH_FACTOR_Y2);
    const glm::ivec3 hashZ_vec3  = glm::ivec3(HASH_FACTOR_Z0, HASH_FACTOR_Z1, HASH_FACTOR_Z2);
    const glm::imat3 hash_mat3x3 = glm::imat3(hashX_vec3, hashY_vec3, hashZ_vec3);

    const glm::ivec4 hashX_vec4  = glm::ivec4(HASH_FACTOR_X0, HASH_FACTOR_X1, HASH_FACTOR_X2, HASH_FACTOR_X3);
    const glm::ivec4 hashY_vec4  = glm::ivec4(HASH_FACTOR_Y0, HASH_FACTOR_Y1, HASH_FACTOR_Y2, HASH_FACTOR_Y3);
    const glm::ivec4 hashZ_vec4  = glm::ivec4(HASH_FACTOR_Z0, HASH_FACTOR_Z1, HASH_FACTOR_Z2, HASH_FACTOR_Z3);
    const glm::ivec4 hashW_vec4  = glm::ivec4(HASH_FACTOR_W0, HASH_FACTOR_W1, HASH_FACTOR_W2, HASH_FACTOR_W3);
    const glm::imat4 hash_mat4x4 = glm::imat4(hashX_vec4, hashY_vec4, hashZ_vec4, hashW_vec4);

    //===================================================================================================================================================================================================================
    // smoothering functions : real_t should be a real type : float or double
    //===================================================================================================================================================================================================================

#if defined(__SSE2__)
    const __m128 _m128_1  = _mm_set1_ps(1.0f);        
    const __m128 _m128_3  = _mm_set1_ps(3.0f);
    const __m128 _m128_6  = _mm_set1_ps(6.0f);
    const __m128 _m128_10 = _mm_set1_ps(10.0f);
    const __m128 _m128_15 = _mm_set1_ps(15.0f);
    const __m128 _m128_20 = _mm_set1_ps(20.0f);
    const __m128 _m128_35 = _mm_set1_ps(35.0f);
    const __m128 _m128_70 = _mm_set1_ps(70.0f);
    const __m128 _m128_84 = _mm_set1_ps(84.0f);

    const __m128d _m128d_1  = _mm_set1_pd(1.0);        
    const __m128d _m128d_3  = _mm_set1_pd(3.0);
    const __m128d _m128d_6  = _mm_set1_pd(6.0);
    const __m128d _m128d_10 = _mm_set1_pd(10.0);
    const __m128d _m128d_15 = _mm_set1_pd(15.0);
    const __m128d _m128d_20 = _mm_set1_pd(20.0);
    const __m128d _m128d_35 = _mm_set1_pd(35.0);
    const __m128d _m128d_70 = _mm_set1_pd(70.0);
    const __m128d _m128d_84 = _mm_set1_pd(84.0);

    const __m128i _m128i_LINEAR_FACTOR_XYZW = _mm_set_epi32(HASH_FACTOR_W, HASH_FACTOR_Z, HASH_FACTOR_Y, HASH_FACTOR_X);
    const __m128i _m128i_LINEAR_SHIFT_0_X_Y_XY = _mm_set_epi32(HASH_FACTOR_X + HASH_FACTOR_Y, HASH_FACTOR_Y, HASH_FACTOR_X, 0);
    const __m128i _m128i_LINEAR_SHIFT_0_Y_X_YX = _mm_set_epi32(HASH_FACTOR_Y + HASH_FACTOR_X, HASH_FACTOR_X, HASH_FACTOR_Y, 0);

    const __m128i _m128i_0101 = _mm_set_epi32(1, 0, 1, 0); 
    const __m128i _m128i_SHIFT_0Y0W = _mm_set_epi32(HASH_FACTOR_W, 0, HASH_FACTOR_Y, 0);
    const __m128i _m128i_SHIFT_XZXZ = _mm_set_epi32(HASH_FACTOR_Z, HASH_FACTOR_X, HASH_FACTOR_Z, HASH_FACTOR_X);
    const __m128i _m128i_HASH_FACTOR_XYXY = _mm_set_epi32(HASH_FACTOR_Y, HASH_FACTOR_X, HASH_FACTOR_Y, HASH_FACTOR_X);
    const __m128i _m128i_HASH_FACTOR_X0Y0 = _mm_set_epi32(0, HASH_FACTOR_Y, 0, HASH_FACTOR_X);
    const __m128i _m128i_HASH_FACTOR_XXYY = _mm_set_epi32(HASH_FACTOR_Y, HASH_FACTOR_Y, HASH_FACTOR_X, HASH_FACTOR_X);
    const __m128i _m128i_HASH_FACTOR = _mm_set_epi32(HASH_FACTOR, HASH_FACTOR, HASH_FACTOR, HASH_FACTOR);
    const __m128i _M128_HASH_SHIFT_X0 = _mm_set_epi32(HASH_SEED, HASH_SEED + HASH_FACTOR_Y, HASH_SEED + HASH_FACTOR_Z, HASH_SEED + HASH_FACTOR_Y + HASH_FACTOR_Z);
    const __m128i _M128_HASH_SHIFT_X1 = _mm_set_epi32(HASH_SEED + HASH_FACTOR_X, HASH_SEED + HASH_FACTOR_X + HASH_FACTOR_Y, HASH_SEED + HASH_FACTOR_X + HASH_FACTOR_Z, HASH_SEED + HASH_FACTOR_X + HASH_FACTOR_Y + HASH_FACTOR_Z);
    const __m128i _M128_HASH_SHIFT_00_01_10_11 = _mm_set_epi32(HASH_SEED, HASH_SEED + HASH_FACTOR_X, HASH_SEED + HASH_FACTOR_Y, HASH_SEED + HASH_FACTOR_X + HASH_FACTOR_Y);
    const __m128d __m128d_F2_HALF = _mm_set_pd(0.5, (glm::sqrt(3.0) - 1.0) / 2.0);
    const __m128d __m128d_G2_G2M1 = _mm_set_pd(-(3.0 + glm::sqrt(3.0)) / 6.0, (3.0 - glm::sqrt(3.0)) / 6.0);

#endif

#if defined(__AVX2__)
    const __m256 _m256_1  = _mm256_set1_ps(1.0);        
    const __m256 _m256_3  = _mm256_set1_ps(3.0);
    const __m256 _m256_6  = _mm256_set1_ps(6.0);
    const __m256 _m256_10 = _mm256_set1_ps(10.0);
    const __m256 _m256_15 = _mm256_set1_ps(15.0);
    const __m256 _m256_20 = _mm256_set1_ps(20.0);
    const __m256 _m256_35 = _mm256_set1_ps(35.0);
    const __m256 _m256_70 = _mm256_set1_ps(70.0);
    const __m256 _m256_84 = _mm256_set1_ps(84.0);

    const __m256d _m256d_1  = _mm256_set1_pd(1.0);        
    const __m256d _m256d_3  = _mm256_set1_pd(3.0);
    const __m256d _m256d_6  = _mm256_set1_pd(6.0);
    const __m256d _m256d_10 = _mm256_set1_pd(10.0);
    const __m256d _m256d_15 = _mm256_set1_pd(15.0);
    const __m256d _m256d_20 = _mm256_set1_pd(20.0);
    const __m256d _m256d_35 = _mm256_set1_pd(35.0);
    const __m256d _m256d_70 = _mm256_set1_pd(70.0);
    const __m256d _m256d_84 = _mm256_set1_pd(84.0);
#endif

    //===================================================================================================================================================================================================================
    // default implementation normalization factors
    //===================================================================================================================================================================================================================
    const double VALUE_2D_NORMALIZATION_FACTOR = 4.6566134281889710651523e-10;
    const double VALUE_3D_NORMALIZATION_FACTOR = 4.6566131506331652780264e-10;
    const double VALUE_4D_NORMALIZATION_FACTOR = 4.6566164813050184803542e-10;

    const double GRADIENT_2D_NORMALIZATION_FACTOR = 5.2075890703651262058723e-10;
    const double GRADIENT_3D_NORMALIZATION_FACTOR = 4.6776896685657035417140e-10;
    const double GRADIENT_4D_NORMALIZATION_FACTOR = 4.7328796069320013945435e-10;

    const double SIMPLEX_2D_NORMALIZATION_FACTOR = 3.323598292361786281372174e-8;
    const double SIMPLEX_3D_NORMALIZATION_FACTOR = 1.160188960440062382968684e-8;
    const double SIMPLEX_4D_NORMALIZATION_FACTOR = 1.145487653796608895744724e-8;

    //===================================================================================================================================================================================================================
    // SSE2 implementation normalization factors
    //===================================================================================================================================================================================================================
    const double VALUE_2D_SSE2_NORMALIZATION_FACTOR = 4.6566787946874146506664e-10;
    const double VALUE_3D_SSE2_NORMALIZATION_FACTOR = 1.0;
    const double VALUE_4D_SSE2_NORMALIZATION_FACTOR = 1.0;

    const double SIMPLEX_2D_SSE2_NORMALIZATION_FACTOR = 1.0;
    const double SIMPLEX_3D_SSE2_NORMALIZATION_FACTOR = 1.0;
    const double SIMPLEX_4D_SSE2_NORMALIZATION_FACTOR = 1.0;

    const double GRADIENT_2D_SSE2_NORMALIZATION_FACTOR = 1.0;
    const double GRADIENT_3D_SSE2_NORMALIZATION_FACTOR = 1.0;
    const double GRADIENT_4D_SSE2_NORMALIZATION_FACTOR = 1.0;

    //===================================================================================================================================================================================================================
    // SSE41 implementation normalization factors
    //===================================================================================================================================================================================================================
    const double VALUE_2D_SSE41_NORMALIZATION_FACTOR = 4.6566787946874146506664e-10;
    const double VALUE_3D_SSE41_NORMALIZATION_FACTOR = 1.0;
    const double VALUE_4D_SSE41_NORMALIZATION_FACTOR = 1.0;
    const double VALUE_4D_SSE41_NORMALIZATION_FACTOR_F = 1.0;

    const double SIMPLEX_2D_SSE41_NORMALIZATION_FACTOR = 3.321322288401046071915217e-8;
    const double SIMPLEX_3D_SSE41_NORMALIZATION_FACTOR = 1.0;
    const double SIMPLEX_4D_SSE41_NORMALIZATION_FACTOR = 1.0;

    const double GRADIENT_2D_SSE41_NORMALIZATION_FACTOR = 5.2842603937570191767512e-10;
    const double GRADIENT_3D_SSE41_NORMALIZATION_FACTOR = 1.0;
    const double GRADIENT_4D_SSE41_NORMALIZATION_FACTOR = 1.0;

    //===================================================================================================================================================================================================================
    // AVX2 implementation normalization factors
    //===================================================================================================================================================================================================================
    const double VALUE_2D_AVX2_NORMALIZATION_FACTOR = 4.6566135561248615396421e-10;
    const double VALUE_3D_AVX2_NORMALIZATION_FACTOR = 1.0;
    const double VALUE_4D_AVX2_NORMALIZATION_FACTOR = 4.6566473249405149794175e-10;

    const double SIMPLEX_2D_AVX2_NORMALIZATION_FACTOR = 3.49896163850663081569615e-9;
    const double SIMPLEX_3D_AVX2_NORMALIZATION_FACTOR = 1.0;
    const double SIMPLEX_4D_AVX2_NORMALIZATION_FACTOR = 1.0;

    const double GRADIENT_4D_AVX2_NORMALIZATION_FACTOR = 4.7946941615309843792241e-10;
    const float  GRADIENT_4D_AVX2_NORMALIZATION_FACTOR_F = 2.1464817975046575535814e-10;
    const double GRADIENT_2D_AVX2_NORMALIZATION_FACTOR = 5.2150278605236472164523e-10;


    //===================================================================================================================================================================================================================
    // bit mixing functions
    //===================================================================================================================================================================================================================
    inline int32_t hash(int32_t prehash)
        { return ((prehash >> HASH_SHIFT) ^ prehash) * HASH_FACTOR; }

    inline glm::ivec2 hash(const glm::ivec2& prehash)
        { return glm::ivec2(hash(prehash.x), hash(prehash.y)); }

    inline glm::ivec3 hash(const glm::ivec3& prehash)
        { return glm::ivec3(hash(prehash.x), hash(prehash.y), hash(prehash.z)); }

    inline glm::ivec4 hash(const glm::ivec4& prehash)
        { return glm::ivec4(hash(prehash.x), hash(prehash.y), hash(prehash.z), hash(prehash.w)); }

    //===================================================================================================================================================================================================================
    // entier(x) = [x] functions
    //===================================================================================================================================================================================================================
    inline int entier(float x) 
        { return (x >= 0.0f ? (int)x : (int)x - 1); }

    inline int entier(double x) 
        { return (x >= 0.0 ? (int)x : (int)x - 1); }

    inline glm::ivec2 entier(const glm::vec2& v) 
        { return glm::ivec2(entier(v.x), entier(v.y)); }

    inline glm::ivec2 entier(const glm::dvec2& v) 
        { return glm::ivec2(entier(v.x), entier(v.y)); }

    inline glm::ivec3 entier(const glm::vec3& v) 
        { return glm::ivec3(entier(v.x), entier(v.y), entier(v.z)); }

    inline glm::ivec3 entier(const glm::dvec3& v) 
        { return glm::ivec3(entier(v.x), entier(v.y), entier(v.z)); }

    inline glm::ivec4 entier(const glm::vec4& v) 
        { return glm::ivec4(entier(v.x), entier(v.y), entier(v.z), entier(v.w)); }

    inline glm::ivec4 entier(const glm::dvec4& v) 
        { return glm::ivec4(entier(v.x), entier(v.y), entier(v.z), entier(v.w)); }

    //===================================================================================================================================================================================================================
    // smoothering functions : real_t should be a real type : float or double
    //===================================================================================================================================================================================================================

    inline float smooth_step3(float x)
        { return x * x * (3.0f - x - x); }

    inline double smooth_step3(double x)
        { return x * x * (3.0 - x - x); }

    inline glm::vec2 smooth_step3(const glm::vec2& v)
        { return v * v * (glm::vec2(3.0f) - v - v); }

    inline glm::dvec2 smooth_step3(const glm::dvec2& v)
        { return v * v * (glm::dvec2(3.0) - v - v); }

    inline glm::vec3 smooth_step3(const glm::vec3& v)
        { return v * v * (glm::vec3(3.0f) - v - v); }

    inline glm::dvec3 smooth_step3(const glm::dvec3& v)
        { return v * v * (glm::dvec3(3.0) - v - v); }

    inline glm::vec4 smooth_step3(const glm::vec4& v)
        { return v * v * (glm::vec4(3.0f) - v - v); }

    inline glm::dvec4 smooth_step3(const glm::dvec4& v)
        { return v * v * (glm::dvec4(3.0) - v - v); }




    inline float smooth_step5(float x)
        { return x * x * x * (10.0f - x * (15.0f - 6.0f * x)); }

    inline double smooth_step5(double x)
        { return x * x * x * (10.0 - x * (15.0 - 6.0 * x)); }

    inline glm::vec2 smooth_step5(const glm::vec2& v)
        { return v * v * v * (glm::vec2(10.0f) - v * (glm::vec2(15.0f) - 6.0f * v)); }

    inline glm::dvec2 smooth_step5(const glm::dvec2& v)
        { return v * v * v * (glm::dvec2(10.0) - v * (glm::dvec2(15.0) - 6.0 * v)); }

    inline glm::vec3 smooth_step5(const glm::vec3& v)
        { return v * v * v * (glm::vec3(10.0f) - v * (glm::vec3(15.0f) - 6.0f * v)); }

    inline glm::dvec3 smooth_step5(const glm::dvec3& v)
        { return v * v * v * (glm::dvec3(10.0) - v * (glm::dvec3(15.0) - 6.0 * v)); }

    inline glm::vec4 smooth_step5(const glm::vec4& v)
        { return v * v * v * (glm::vec4(10.0f) - v * (glm::vec4(15.0f) - 6.0f * v)); }

    inline glm::dvec4 smooth_step5(const glm::dvec4& v)
        { return v * v * v * (glm::dvec4(10.0) - v * (glm::dvec4(15.0) - 6.0 * v)); }



    inline float smooth_step7(float x)
    {
        float sq = x * x;
        return sq * sq * (35.0f - x * (84.0f - x * (70.0f - 20.0f * x)));
    }

    inline double smooth_step7(double x)
    {
        double sq = x * x;
        return sq * sq * (35.0 - x * (84.0 - x * (70.0 - 20.0 * x)));
    }

    inline glm::vec2 smooth_step7(const glm::vec2& v)
    {
        glm::vec2 sq = v * v;
        return sq * sq * (glm::vec2(35.0f) - v * (glm::vec2(84.0f) - v * (glm::vec2(70.0f) - 20.0f * v)));
    }

    inline glm::dvec2 smooth_step7(const glm::dvec2& v)
    {
        glm::dvec2 sq = v * v;
        return sq * sq * (glm::dvec2(35.0) - v * (glm::dvec2(84.0) - v * (glm::dvec2(70.0) - 20.0 * v)));
    }

    inline glm::vec3 smooth_step7(const glm::vec3& v)
    {
        glm::vec3 sq = v * v;
        return sq * sq * (glm::vec3(35.0f) - v * (glm::vec3(84.0f) - v * (glm::vec3(70.0f) - 20.0f * v)));
    }

    inline glm::dvec3 smooth_step7(const glm::dvec3& v)
    {
        glm::dvec3 sq = v * v;
        return sq * sq * (glm::dvec3(35.0) - v * (glm::dvec3(84.0) - v * (glm::dvec3(70.0) - 20.0 * v)));
    }

    inline glm::vec4 smooth_step7(const glm::vec4& v)
    {
        glm::vec4 sq = v * v;
        return sq * sq * (glm::vec4(35.0f) - v * (glm::vec4(84.0f) - v * (glm::vec4(70.0f) - 20.0f * v)));
    }

    inline glm::dvec4 smooth_step7(const glm::dvec4& v)
    {
        glm::dvec4 sq = v * v;
        return sq * sq * (glm::dvec4(35.0) - v * (glm::dvec4(84.0) - v * (glm::dvec4(70.0) - 20.0 * v)));
    }

#if defined(__SSE2__)
    inline __m128 smooth_step3(const __m128& t)
        { return _mm_mul_ps(t, _mm_mul_ps(t, _mm_sub_ps(_m128_3, _mm_add_ps(t, t)))); }

    inline __m128 smooth_step5(const __m128& t)
        { return _mm_mul_ps(t, _mm_mul_ps(t, _mm_mul_ps(t, _mm_sub_ps(_m128_10, _mm_mul_ps(t, _mm_sub_ps(_m128_15, _mm_mul_ps(t, _m128_6))))))); }

    inline __m128 smooth_step7(const __m128& t)
    {
        __m128 sq = _mm_mul_ps(t, t);
        __m128 p4 = _mm_mul_ps(sq, sq);
        return _mm_mul_ps(p4, _mm_sub_ps(_m128_35, _mm_mul_ps(t, _mm_sub_ps(_m128_84, _mm_mul_ps(t, _mm_sub_ps(_m128_70, _mm_mul_ps(t, _m128_20)))))));
    }

    inline __m128d smooth_step3(const __m128d& t)
        { return _mm_mul_pd(t, _mm_mul_pd(t, _mm_sub_pd(_m128d_3, _mm_add_pd(t, t)))); }

    inline __m128d smooth_step5(const __m128d& t)
        { return _mm_mul_pd(t, _mm_mul_pd(t, _mm_mul_pd(t, _mm_sub_pd(_m128d_10, _mm_mul_pd(t, _mm_sub_pd(_m128d_15, _mm_mul_pd(t, _m128d_6))))))); }

    inline __m128d smooth_step7(const __m128d& t)
    {
        __m128d sq = _mm_mul_pd(t, t);
        __m128d p4 = _mm_mul_pd(sq, sq);
        return _mm_mul_pd(p4, _mm_sub_pd(_m128d_35, _mm_mul_pd(t, _mm_sub_pd(_m128d_84, _mm_mul_pd(t, _mm_sub_pd(_m128d_70, _mm_mul_pd(t, _m128d_20)))))));
    }
#endif

#if defined(__AVX2__)
    inline __m256 smooth_step3(const __m256& t)
        { return _mm256_mul_ps(t, _mm256_mul_ps(t, _mm256_sub_ps(_m256_3, _mm256_add_ps(t, t)))); }

    inline __m256 smooth_step5(const __m256& t)
        { return _mm256_mul_ps(t, _mm256_mul_ps(t, _mm256_mul_ps(t, _mm256_sub_ps(_m256_10, _mm256_mul_ps(t, _mm256_sub_ps(_m256_15, _mm256_mul_ps(t, _m256_6))))))); }

    inline __m256 smooth_step7(const __m256& t)
    {
        __m256 sq = _mm256_mul_ps(t, t);
        __m256 p4 = _mm256_mul_ps(sq, sq);
        return _mm256_mul_ps(p4, _mm256_sub_ps(_m256_35, _mm256_mul_ps(t, _mm256_sub_ps(_m256_84, _mm256_mul_ps(t, _mm256_sub_ps(_m256_70, _mm256_mul_ps(t, _m256_20)))))));
    }

    inline __m256d smooth_step3(const __m256d& t)
        { return _mm256_mul_pd(t, _mm256_mul_pd(t, _mm256_sub_pd(_m256d_3, _mm256_add_pd(t, t)))); }

    inline __m256d smooth_step5(const __m256d& t)
        { return _mm256_mul_pd(t, _mm256_mul_pd(t, _mm256_mul_pd(t, _mm256_sub_pd(_m256d_10, _mm256_mul_pd(t, _mm256_sub_pd(_m256d_15, _mm256_mul_pd(t, _m256d_6))))))); }

    inline __m256d smooth_step7(const __m256d& t)
    {
        __m256d sq = _mm256_mul_pd(t, t);
        __m256d p4 = _mm256_mul_pd(sq, sq);
        return _mm256_mul_pd(p4, _mm256_sub_pd(_m256d_35, _mm256_mul_pd(t, _mm256_sub_pd(_m256d_84, _mm256_mul_pd(t, _mm256_sub_pd(_m256d_70, _mm256_mul_pd(t, _m256d_20)))))));
    }
#endif

} // namespace noise

#endif // _noise_common_included_345627435978435627843659784356824375678239465237