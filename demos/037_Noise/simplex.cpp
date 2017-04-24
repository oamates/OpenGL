#include <glm/gtx/norm.hpp>

#include "value.hpp"
#include "common.hpp"

namespace noise {

    const double sqrt3_d = 1.732050807568877293527446341505872366942805253810380628055;
    const double F2 = (sqrt3_d - 1.0) / 2.0;
    const double G2_1 = (3.0 - sqrt3_d) / 6.0;
    const double G2_2 = 2.0 * G2_1 - 1.0;

    const double F3 = 1.0 / 3.0;
    const double G3_1 = 1.0 / 6.0;
    const double G3_2 = 2.0 * G3_1;
    const double G3_3 = 3.0 * G3_1 - 1.0;

    const double sqrt5_d = 2.236067977499789696409173668731276235440618359611525724270;
    const double F4 = (sqrt5_d - 1.0) / 4.0;
    const double G4_1 = (5.0 - sqrt5_d) / 20.0;
    const double G4_2 = 2.0 * G4_1;
    const double G4_3 = 3.0 * G4_1;
    const double G4_4 = 4.0 * G4_1 - 1.0;
          
          
    //===================================================================================================================================================================================================================
    // simplex 2d noise
    //===================================================================================================================================================================================================================
    template<typename real_t> real_t simplex(const glm::tvec2<real_t>& P)
    {
        real_t t = (P.x + P.y) * F2;
        glm::ivec2 Pi = entier(P + glm::tvec2<real_t>(t));
        t = (Pi.x + Pi.y) * G2_1;

        glm::tvec2<real_t> P0 = glm::tvec2<real_t>(Pi) - glm::tvec2<real_t>(t);
        glm::tvec2<real_t> p0 = P - P0;
        glm::tvec2<real_t> p1 = p0 + glm::tvec2<real_t>(G2_1);
        glm::tvec2<real_t> p2 = p0 + glm::tvec2<real_t>(G2_2);

        glm::ivec2 hash0 = hash_mat2x2 * Pi;
        glm::ivec2 hash1 = hash0;
        glm::ivec2 hash2 = hash0 + hashX_vec2 + hashY_vec2;
        
        if (p0.x > p0.y)
            { p1.x -= 1.0; hash1 += hashX_vec2; }
        else
            { p1.y -= 1.0; hash1 += hashY_vec2; }

        hash0 = hash(hash0);
        hash1 = hash(hash1);
        hash2 = hash(hash2);
        
        real_t s = 0.0;
        
        t = 0.5 - glm::length2(p0);
        if (t > 0.0)
            { t *= t; s += t * t * glm::dot(p0, glm::tvec2<real_t>(hash0)); }
        
        t = 0.5 - glm::length2(p1);
        if (t > 0.0)
            { t *= t; s += t * t * glm::dot(p1, glm::tvec2<real_t>(hash1)); }
        
        t = 0.5 - glm::length2(p2);
        if (t > 0.0)
            { t *= t; s += t * t * glm::dot(p2, glm::tvec2<real_t>(hash2)); }

        return SIMPLEX_2D_NORMALIZATION_FACTOR * s;
    }

    template<typename real_t> real_t simplex_fbm4(const glm::tvec2<real_t>& P, const glm::tmat2x2<real_t>* matrices, const glm::tvec2<real_t>* shifts, const real_t* amplitudes)
    {
        real_t v = 0.0;
        for(int i = 0; i < 4; ++i)
        {
            glm::tvec2<real_t> PP = shifts[i] + matrices[i] * P;
            v += amplitudes[i] * simplex<real_t>(PP);
        }
        return v;
    }


    //===================================================================================================================================================================================================================
    // simplex 3d noise
    //===================================================================================================================================================================================================================
    template<typename real_t> real_t simplex(const glm::tvec3<real_t>& P)
    {
        real_t t = (P.x + P.y + P.z) * F3;
        glm::ivec3 Pi = entier(P + glm::tvec3<real_t>(t));
        t = (Pi.x + Pi.y + Pi.z) * G3_1;

        glm::tvec3<real_t> P0 = glm::tvec3<real_t>(Pi) - glm::tvec3<real_t>(t);
        glm::tvec3<real_t> p0 = P - P0;
        glm::tvec3<real_t> p1 = p0 + glm::tvec3<real_t>(G3_1);
        glm::tvec3<real_t> p2 = p0 + glm::tvec3<real_t>(G3_2);
        glm::tvec3<real_t> p3 = p0 + glm::tvec3<real_t>(G3_3);

        glm::ivec3 hash0 = hash_mat3x3 * Pi;
        glm::ivec3 hash1, hash2;
        glm::ivec3 hash3 = hash0 + hashX_vec3 + hashY_vec3 + hashZ_vec3;

        if (p0.x >= p0.y)
        {
            if (p0.y >= p0.z)                                                           // x > y > z
            {
                p1.x -= 1.0; p2.x -= 1.0; p2.y -= 1.0;
                hash1 = hash0 + hashX_vec3;
                hash2 = hash1 + hashY_vec3; 
            }
            else if (p0.x >= p0.z)                                                      // x > z > y
            {
                p1.x -= 1.0; p2.x -= 1.0; p2.z -= 1.0; 
                hash1 = hash0 + hashX_vec3; 
                hash2 = hash1 + hashZ_vec3;
            }
            else                                                                        // z > x > y
            {
                p1.z -= 1.0; p2.z -= 1.0; p2.x -= 1.0; 
                hash1 = hash0 + hashZ_vec3;
                hash2 = hash1 + hashX_vec3;
            }
        }
        else
        {
            if (p0.y < p0.z)                                                            // z > y > x
            { 
                p1.z -= 1.0; p2.z -= 1.0; p2.y -= 1.0; 
                hash1 = hash0 + hashZ_vec3;
                hash2 = hash1 + hashY_vec3;
            }
            else if (p0.x < p0.z)                                                       // y > z > x
            {
                p1.y -= 1.0; p2.y -= 1.0; p2.z -= 1.0; 
                hash1 = hash0 + hashY_vec3;
                hash2 = hash1 + hashZ_vec3;
            }
            else                                                                        // y > x > z
            {
                p1.y -= 1.0; p2.y -= 1.0; p2.x -= 1.0; 
                hash1 = hash0 + hashY_vec3;
                hash2 = hash1 + hashX_vec3;
            }                                                            
        }

        hash0 = hash(hash0);
        hash1 = hash(hash1);
        hash2 = hash(hash2);
        hash3 = hash(hash3);

        real_t s = 0.0;
        
        t = 0.6 - glm::length2(p0);
        if (t > 0.0)
            { t *= t; s += t * t * glm::dot(p0, glm::tvec3<real_t>(hash0)); }
        
        t = 0.6 - glm::length2(p1);
        if (t > 0.0)
            { t *= t; s += t * t * glm::dot(p1, glm::tvec3<real_t>(hash1)); }
        
        t = 0.6 - glm::length2(p2);
        if (t > 0.0)
            { t *= t; s += t * t * glm::dot(p2, glm::tvec3<real_t>(hash2)); }
        
        t = 0.6 - glm::length2(p3);
        if (t > 0.0)
            { t *= t; s += t * t * glm::dot(p3, glm::tvec3<real_t>(hash3)); }
        
        return SIMPLEX_3D_NORMALIZATION_FACTOR * s;
    }

    template<typename real_t> real_t simplex_fbm4(const glm::tvec3<real_t>& P, const glm::tmat3x3<real_t>* matrices, const glm::tvec3<real_t>* shifts, const real_t* amplitudes)
    {
        real_t v = 0.0;
        for(int i = 0; i < 4; ++i)
        {
            glm::tvec3<real_t> PP = shifts[i] + matrices[i] * P;
            v += amplitudes[i] * simplex<real_t>(PP);
        }
        return v;
    }

    //===================================================================================================================================================================================================================
    // simplex 4d noise
    //===================================================================================================================================================================================================================
    template<typename real_t> real_t simplex(const glm::tvec4<real_t>& P)
    {
        real_t t = (P.x + P.y + P.z + P.w) * F4;
        glm::ivec4 Pi = entier(P + glm::tvec4<real_t>(t));
        t = (Pi.x + Pi.y + Pi.z + Pi.w) * G4_1;

        glm::tvec4<real_t> P0 = glm::tvec4<real_t>(Pi) - glm::tvec4<real_t>(t);
        glm::tvec4<real_t> p0 = P - P0;
        
        glm::tvec4<real_t> p1 = p0 + glm::tvec4<real_t>(G4_1);
        glm::tvec4<real_t> p2 = p0 + glm::tvec4<real_t>(G4_2);
        glm::tvec4<real_t> p3 = p0 + glm::tvec4<real_t>(G4_3);
        glm::tvec4<real_t> p4 = p0 + glm::tvec4<real_t>(G4_4);

        glm::ivec4 hash0 = hash_mat4x4 * Pi;
        glm::ivec4 hash1, hash2, hash3;
        glm::ivec4 hash4 = hash0 + hashX_vec4 + hashY_vec4 + hashZ_vec4 + hashW_vec4;

        if (p0.x >= p0.y)                                                               // x > y
        {
            if (p0.y >= p0.z)                                                           // x > y > z
            {
                if (p0.z >= p0.w)                                                       // x > y > z > w
                {
                    p1.x -= 1.0; p2.x -= 1.0; p3.x -= 1.0; p2.y -= 1.0; p3.y -= 1.0; p3.z -= 1.0;
                    hash1 = hash0 + hashX_vec4;
                    hash2 = hash1 + hashY_vec4; 
                    hash3 = hash2 + hashZ_vec4; 
                }
                else if (p0.y >= p0.w)                                                  // x > y > w > z
                {
                    p1.x -= 1.0; p2.x -= 1.0; p3.x -= 1.0; p2.y -= 1.0; p3.y -= 1.0; p3.w -= 1.0;
                    hash1 = hash0 + hashX_vec4;
                    hash2 = hash1 + hashY_vec4; 
                    hash3 = hash2 + hashW_vec4; 
                }
                else if (p0.x >= p0.w)                                                  // x > w > y > z
                {
                    p1.x -= 1.0; p2.x -= 1.0; p3.x -= 1.0; p2.w -= 1.0; p3.w -= 1.0; p3.y -= 1.0;
                    hash1 = hash0 + hashX_vec4;
                    hash2 = hash1 + hashW_vec4; 
                    hash3 = hash2 + hashY_vec4; 
                }
                else                                                                    // w > x > y > z
                {
                    p1.w -= 1.0; p2.w -= 1.0; p3.w -= 1.0; p2.x -= 1.0; p3.x -= 1.0; p3.y -= 1.0;
                    hash1 = hash0 + hashW_vec4;
                    hash2 = hash1 + hashX_vec4; 
                    hash3 = hash2 + hashY_vec4; 
                }
            }
            else if (p0.x >= p0.z)                                                      // x > z > y
            {
                if (p0.y >= p0.w)                                                       // x > z > y > w
                {
                    p1.x -= 1.0; p2.x -= 1.0; p3.x -= 1.0; p2.z -= 1.0; p3.z -= 1.0; p3.y -= 1.0;
                    hash1 = hash0 + hashX_vec4;
                    hash2 = hash1 + hashZ_vec4; 
                    hash3 = hash2 + hashY_vec4; 
                }
                else if (p0.z >= p0.w)                                                  // x > z > w > y
                {
                    p1.x -= 1.0; p2.x -= 1.0; p3.x -= 1.0; p2.z -= 1.0; p3.z -= 1.0; p3.w -= 1.0;
                    hash1 = hash0 + hashX_vec4;
                    hash2 = hash1 + hashZ_vec4; 
                    hash3 = hash2 + hashW_vec4; 
                }
                else if (p0.x >= p0.w)                                                  // x > w > z > y
                {
                    p1.x -= 1.0; p2.x -= 1.0; p3.x -= 1.0; p2.w -= 1.0; p3.w -= 1.0; p3.z -= 1.0;
                    hash1 = hash0 + hashX_vec4;
                    hash2 = hash1 + hashW_vec4; 
                    hash3 = hash2 + hashZ_vec4; 
                }
                else                                                                    // w > x > z > y
                {
                    p1.w -= 1.0; p2.w -= 1.0; p3.w -= 1.0; p2.x -= 1.0; p3.x -= 1.0; p3.z -= 1.0;
                    hash1 = hash0 + hashW_vec4;
                    hash2 = hash1 + hashX_vec4; 
                    hash3 = hash2 + hashZ_vec4; 
                }
            }
            else                                                                        // z > x > y
            {
                if (p0.y >= p0.w)                                                       // z > x > y > w
                {
                    p1.z -= 1.0; p2.z -= 1.0; p3.z -= 1.0; p2.x -= 1.0; p3.x -= 1.0; p3.y -= 1.0;
                    hash1 = hash0 + hashZ_vec4;
                    hash2 = hash1 + hashX_vec4; 
                    hash3 = hash2 + hashY_vec4; 
                }
                else if (p0.x >= p0.w)                                                  // z > x > w > y
                {
                    p1.z -= 1.0; p2.z -= 1.0; p3.z -= 1.0; p2.x -= 1.0; p3.x -= 1.0; p3.w -= 1.0;
                    hash1 = hash0 + hashZ_vec4;
                    hash2 = hash1 + hashX_vec4; 
                    hash3 = hash2 + hashW_vec4; 
                }
                else if (p0.z >= p0.w)                                                  // z > w > x > y
                {
                    p1.z -= 1.0; p2.z -= 1.0; p3.z -= 1.0; p2.w -= 1.0; p3.w -= 1.0; p3.x -= 1.0;
                    hash1 = hash0 + hashZ_vec4;
                    hash2 = hash1 + hashW_vec4; 
                    hash3 = hash2 + hashX_vec4; 
                }
                else                                                                    // w > z > x > y
                {
                    p1.w -= 1.0; p2.w -= 1.0; p3.w -= 1.0; p2.z -= 1.0; p3.z -= 1.0; p3.x -= 1.0;
                    hash1 = hash0 + hashW_vec4;
                    hash2 = hash1 + hashZ_vec4; 
                    hash3 = hash2 + hashX_vec4; 
                }
            }
        }
        else                                                                            // y > x
        {
            if (p0.y < p0.z)                                                            // z > y > x
            { 
                if (p0.w < p0.x)                                                        // z > y > x > w
                {
                    p1.z -= 1.0; p2.z -= 1.0; p3.z -= 1.0; p2.y -= 1.0; p3.y -= 1.0; p3.x -= 1.0;
                    hash1 = hash0 + hashZ_vec4;
                    hash2 = hash1 + hashY_vec4; 
                    hash3 = hash2 + hashX_vec4; 
                }
                else if (p0.w < p0.y)                                                   // z > y > w > x
                {
                    p1.z -= 1.0; p2.z -= 1.0; p3.z -= 1.0; p2.y -= 1.0; p3.y -= 1.0; p3.w -= 1.0;
                    hash1 = hash0 + hashZ_vec4;
                    hash2 = hash1 + hashY_vec4; 
                    hash3 = hash2 + hashW_vec4; 
                }
                else if (p0.w < p0.z)                                                   // z > w > y > x
                {
                    p1.z -= 1.0; p2.z -= 1.0; p3.z -= 1.0; p2.w -= 1.0; p3.w -= 1.0; p3.y -= 1.0;
                    hash1 = hash0 + hashZ_vec4;
                    hash2 = hash1 + hashW_vec4; 
                    hash3 = hash2 + hashY_vec4; 
                }
                else                                                                    // w > z > y > x
                {
                    p1.w -= 1.0; p2.w -= 1.0; p3.w -= 1.0; p2.z -= 1.0; p3.z -= 1.0; p3.y -= 1.0;
                    hash1 = hash0 + hashW_vec4;
                    hash2 = hash1 + hashZ_vec4; 
                    hash3 = hash2 + hashY_vec4; 
                }
            }
            else if (p0.x < p0.z)                                                       // y > z > x
            {
                if (p0.w < p0.x)                                                        // y > z > x > w
                {
                    p1.y -= 1.0; p2.y -= 1.0; p3.y -= 1.0; p2.z -= 1.0; p3.z -= 1.0; p3.x -= 1.0;
                    hash1 = hash0 + hashY_vec4;
                    hash2 = hash1 + hashZ_vec4; 
                    hash3 = hash2 + hashX_vec4; 
                }
                else if (p0.w < p0.z)                                                   // y > z > w > x
                {
                    p1.y -= 1.0; p2.y -= 1.0; p3.y -= 1.0; p2.z -= 1.0; p3.z -= 1.0; p3.w -= 1.0;
                    hash1 = hash0 + hashY_vec4;
                    hash2 = hash1 + hashZ_vec4; 
                    hash3 = hash2 + hashW_vec4; 
                }
                else if (p0.w < p0.y)                                                   // y > w > z > x
                {
                    p1.y -= 1.0; p2.y -= 1.0; p3.y -= 1.0; p2.w -= 1.0; p3.w -= 1.0; p3.z -= 1.0;
                    hash1 = hash0 + hashY_vec4;
                    hash2 = hash1 + hashW_vec4; 
                    hash3 = hash2 + hashZ_vec4; 
                }
                else                                                                    // w > y > z > x
                {
                    p1.w -= 1.0; p2.w -= 1.0; p3.w -= 1.0; p2.y -= 1.0; p3.y -= 1.0; p3.z -= 1.0;
                    hash1 = hash0 + hashW_vec4;
                    hash2 = hash1 + hashY_vec4; 
                    hash3 = hash2 + hashZ_vec4; 
                }
            }
            else                                                                        // y > x > z
            {
                if (p0.w < p0.z)                                                        // y > x > z > w
                {
                    p1.y -= 1.0; p2.y -= 1.0; p3.y -= 1.0; p2.x -= 1.0; p3.x -= 1.0; p3.z -= 1.0;
                    hash1 = hash0 + hashY_vec4;
                    hash2 = hash1 + hashX_vec4; 
                    hash3 = hash2 + hashZ_vec4; 
                }
                else if (p0.w < p0.x)                                                   // y > x > w > z
                {
                    p1.y -= 1.0; p2.y -= 1.0; p3.y -= 1.0; p2.x -= 1.0; p3.x -= 1.0; p3.w -= 1.0;
                    hash1 = hash0 + hashY_vec4;
                    hash2 = hash1 + hashX_vec4; 
                    hash3 = hash2 + hashW_vec4; 
                }
                else if (p0.w < p0.y)                                                   // y > w > x > z
                {
                    p1.y -= 1.0; p2.y -= 1.0; p3.y -= 1.0; p2.w -= 1.0; p3.w -= 1.0; p3.x -= 1.0;
                    hash1 = hash0 + hashY_vec4;
                    hash2 = hash1 + hashW_vec4; 
                    hash3 = hash2 + hashX_vec4; 
                }
                else                                                                    // w > y > x > z
                {
                    p1.w -= 1.0; p2.w -= 1.0; p3.w -= 1.0; p2.y -= 1.0; p3.y -= 1.0; p3.x -= 1.0;
                    hash1 = hash0 + hashW_vec4;
                    hash2 = hash1 + hashY_vec4; 
                    hash3 = hash2 + hashX_vec4; 
                }
            }                                                            
        }

        hash0 = hash(hash0);
        hash1 = hash(hash1);
        hash2 = hash(hash2);
        hash3 = hash(hash3);
        hash4 = hash(hash4);

        real_t s = 0.0;

        t = 0.6 - glm::length2(p0);
        if (t > 0.0)
            { t *= t; s += t * t * glm::dot(p0, glm::tvec4<real_t>(hash0)); }

        t = 0.6 - glm::length2(p1);
        if (t > 0.0)
            { t *= t; s += t * t * glm::dot(p1, glm::tvec4<real_t>(hash1)); }

        t = 0.6 - glm::length2(p2);
        if (t > 0.0)
            { t *= t; s += t * t * glm::dot(p2, glm::tvec4<real_t>(hash2)); }

        t = 0.6 - glm::length2(p3);
        if (t > 0.0)
            { t *= t; s += t * t * glm::dot(p3, glm::tvec4<real_t>(hash3)); }

        t = 0.6 - glm::length2(p4);
        if (t > 0.0)
            { t *= t; s += t * t * glm::dot(p4, glm::tvec4<real_t>(hash4)); }

        return SIMPLEX_4D_NORMALIZATION_FACTOR * s;
    }

    template<typename real_t> real_t simplex_fbm4(const glm::tvec4<real_t>& P, const glm::tmat4x4<real_t>* matrices, const glm::tvec4<real_t>* shifts, const real_t* amplitudes)
    {
        real_t v = 0.0;
        for(int i = 0; i < 4; ++i)
        {
            glm::tvec4<real_t> PP = shifts[i] + matrices[i] * P;
            v += amplitudes[i] * simplex<real_t>(PP);
        }
        return v;
    }

    //===================================================================================================================================================================================================================
    // instantiate above template functions for real_t = float and real_t = double
    //===================================================================================================================================================================================================================

    template float  simplex<float> (const glm::vec2& P);
    template double simplex<double>(const glm::dvec2& P);
    template float  simplex<float> (const glm::vec3& P);
    template double simplex<double>(const glm::dvec3& P);
    template float  simplex<float> (const glm::vec4& P);
    template double simplex<double>(const glm::dvec4& P);

    template float  simplex_fbm4<float> (const glm::vec2&,  const glm::mat2*,  const glm::vec2*,  const float*);
    template double simplex_fbm4<double>(const glm::dvec2&, const glm::dmat2*, const glm::dvec2*, const double*);
    template float  simplex_fbm4<float> (const glm::vec3&,  const glm::mat3*,  const glm::vec3*,  const float*);
    template double simplex_fbm4<double>(const glm::dvec3&, const glm::dmat3*, const glm::dvec3*, const double*);
    template float  simplex_fbm4<float> (const glm::vec4&,  const glm::mat4*,  const glm::vec4*,  const float*);
    template double simplex_fbm4<double>(const glm::dvec4&, const glm::dmat4*, const glm::dvec4*, const double*);

} // namespace noise
