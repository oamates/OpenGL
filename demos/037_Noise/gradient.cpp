#include "gradient.hpp"
#include "common.hpp"

namespace noise {

    //===================================================================================================================================================================================================================
    // 2d gradient noise
    //===================================================================================================================================================================================================================
    template<typename real_t> real_t gradient(const glm::tvec2<real_t>& P)
    {
        glm::ivec2 Pi = entier(P);
        glm::tvec2<real_t> dP0 = P - glm::tvec2<real_t>(Pi);
        glm::tvec2<real_t> dP1 = dP0 - glm::tvec2<real_t>(1.0);
        glm::tvec2<real_t> s = smooth_step5(dP0); 

        glm::ivec2 hash00 = hash_mat2x2 * Pi; 
        glm::ivec2 hash10 = hash00 + hashX_vec2;
        glm::ivec2 hash01 = hash00 + hashY_vec2;
        glm::ivec2 hash11 = hash10 + hashY_vec2;

        hash00 = hash(hash00);
        hash10 = hash(hash10);
        hash01 = hash(hash01);
        hash11 = hash(hash11);
        
        real_t grad00 = hash00.x * dP0.x + hash00.y * dP0.y;
        real_t grad10 = hash10.x * dP1.x + hash10.y * dP0.y;
        real_t grad01 = hash01.x * dP0.x + hash01.y * dP1.y;
        real_t grad11 = hash11.x * dP1.x + hash11.y * dP1.y;

        real_t grad_y0 = grad00 + (grad10 - grad00) * s.x;
        real_t grad_y1 = grad01 + (grad11 - grad01) * s.x;
        
        real_t g = grad_y0 + (grad_y1 - grad_y0) * s.y;
        
        return GRADIENT_2D_NORMALIZATION_FACTOR * g;
    }

    template<typename real_t> real_t gradient_fbm4(const glm::tvec2<real_t>& P, const glm::tmat2x2<real_t>* matrices, const glm::tvec2<real_t>* shifts, const real_t* amplitudes)
    {
        real_t v = 0.0;
        for(int i = 0; i < 4; ++i)
        {
            glm::tvec2<real_t> PP = shifts[i] + matrices[i] * P;
            v += amplitudes[i] * gradient<real_t>(PP);
        }
        return v;
    }

    //===================================================================================================================================================================================================================
    // 3d gradient noise
    //===================================================================================================================================================================================================================
    template<typename real_t> real_t gradient(const glm::tvec3<real_t>& P)
    {
        glm::ivec3 Pi = entier(P);
        glm::tvec3<real_t> dP0 = P - glm::tvec3<real_t>(Pi);
        glm::tvec3<real_t> dP1 = dP0 - glm::tvec3<real_t>(1.0);
        glm::tvec3<real_t> s = smooth_step5(dP0); 

        glm::ivec3 hash000 = hash_mat3x3 * Pi;
        glm::ivec3 hash100 = hash000 + hashX_vec3;
        glm::ivec3 hash010 = hash000 + hashY_vec3;
        glm::ivec3 hash110 = hash100 + hashY_vec3;
        glm::ivec3 hash001 = hash000 + hashZ_vec3;
        glm::ivec3 hash101 = hash100 + hashZ_vec3;
        glm::ivec3 hash011 = hash010 + hashZ_vec3;
        glm::ivec3 hash111 = hash110 + hashZ_vec3;

        hash000 = hash(hash000); 
        hash100 = hash(hash100); 
        hash010 = hash(hash010); 
        hash110 = hash(hash110); 
        hash001 = hash(hash001); 
        hash101 = hash(hash101); 
        hash011 = hash(hash011); 
        hash111 = hash(hash111); 
        
        real_t grad000 = hash000.x * dP0.x + hash000.y * dP0.y + hash000.z * dP0.z;
        real_t grad100 = hash100.x * dP1.x + hash100.y * dP0.y + hash100.z * dP0.z;
        real_t grad010 = hash010.x * dP0.x + hash010.y * dP1.y + hash010.z * dP0.z;
        real_t grad110 = hash110.x * dP1.x + hash110.y * dP1.y + hash110.z * dP0.z;
        real_t grad001 = hash001.x * dP0.x + hash001.y * dP0.y + hash001.z * dP1.z;
        real_t grad101 = hash101.x * dP1.x + hash101.y * dP0.y + hash101.z * dP1.z;
        real_t grad011 = hash011.x * dP0.x + hash011.y * dP1.y + hash011.z * dP1.z;
        real_t grad111 = hash111.x * dP1.x + hash111.y * dP1.y + hash111.z * dP1.z;

        real_t grad_yz00 = grad000 + (grad100 - grad000) * s.x;
        real_t grad_yz10 = grad010 + (grad110 - grad010) * s.x;
        real_t grad_yz01 = grad001 + (grad101 - grad001) * s.x;
        real_t grad_yz11 = grad011 + (grad111 - grad011) * s.x;
        
        real_t grad_z0 = grad_yz00 + (grad_yz10 - grad_yz00) * s.y;
        real_t grad_z1 = grad_yz01 + (grad_yz11 - grad_yz01) * s.y;

        real_t g = grad_z0 + (grad_z1 - grad_z0) * s.z;
        
        return GRADIENT_3D_NORMALIZATION_FACTOR * g;
    }

    template<typename real_t> real_t gradient_fbm4(const glm::tvec3<real_t>& P, const glm::tmat3x3<real_t>* matrices, const glm::tvec3<real_t>* shifts, const real_t* amplitudes)
    {
        real_t v = 0.0;
        for(int i = 0; i < 4; ++i)
        {
            glm::tvec3<real_t> PP = shifts[i] + matrices[i] * P;
            v += amplitudes[i] * gradient<real_t>(PP);
        }
        return v;
    }

    //===================================================================================================================================================================================================================
    // 4d value noise
    //===================================================================================================================================================================================================================

    template<typename real_t> real_t gradient(const glm::tvec4<real_t>& P)
    {
        glm::ivec4 Pi = entier(P);
        glm::tvec4<real_t> dP0 = P - glm::tvec4<real_t>(Pi);
        glm::tvec4<real_t> dP1 = dP0 - glm::tvec4<real_t>(1.0);
        glm::tvec4<real_t> s = smooth_step5(dP0); 

        glm::ivec4 hash0000 = hash_mat4x4 * Pi;
        glm::ivec4 hash1000 = hash0000 + hashX_vec4;
        glm::ivec4 hash0100 = hash0000 + hashY_vec4;
        glm::ivec4 hash1100 = hash1000 + hashY_vec4;
        glm::ivec4 hash0010 = hash0000 + hashZ_vec4;
        glm::ivec4 hash1010 = hash1000 + hashZ_vec4;
        glm::ivec4 hash0110 = hash0100 + hashZ_vec4;
        glm::ivec4 hash1110 = hash1100 + hashZ_vec4;
        glm::ivec4 hash0001 = hash0000 + hashW_vec4;
        glm::ivec4 hash1001 = hash1000 + hashW_vec4;
        glm::ivec4 hash0101 = hash0100 + hashW_vec4;
        glm::ivec4 hash1101 = hash1100 + hashW_vec4;
        glm::ivec4 hash0011 = hash0010 + hashW_vec4;
        glm::ivec4 hash1011 = hash1010 + hashW_vec4;
        glm::ivec4 hash0111 = hash0110 + hashW_vec4;
        glm::ivec4 hash1111 = hash1110 + hashW_vec4;
                                               
        hash0000 = hash(hash0000);
        hash1000 = hash(hash1000);
        hash0100 = hash(hash0100);
        hash1100 = hash(hash1100);
        hash0010 = hash(hash0010);
        hash1010 = hash(hash1010);
        hash0110 = hash(hash0110);
        hash1110 = hash(hash1110);
        hash0001 = hash(hash0001);
        hash1001 = hash(hash1001);
        hash0101 = hash(hash0101);
        hash1101 = hash(hash1101);
        hash0011 = hash(hash0011);
        hash1011 = hash(hash1011);
        hash0111 = hash(hash0111);
        hash1111 = hash(hash1111);

        real_t grad0000 = hash0000.x * dP0.x + hash0000.y * dP0.y + hash0000.z * dP0.z + hash0000.w * dP0.w;
        real_t grad1000 = hash1000.x * dP1.x + hash1000.y * dP0.y + hash1000.z * dP0.z + hash1000.w * dP0.w;
        real_t grad0100 = hash0100.x * dP0.x + hash0100.y * dP1.y + hash0100.z * dP0.z + hash0100.w * dP0.w;
        real_t grad1100 = hash1100.x * dP1.x + hash1100.y * dP1.y + hash1100.z * dP0.z + hash1100.w * dP0.w;
        real_t grad0010 = hash0010.x * dP0.x + hash0010.y * dP0.y + hash0010.z * dP1.z + hash0010.w * dP0.w;
        real_t grad1010 = hash1010.x * dP1.x + hash1010.y * dP0.y + hash1010.z * dP1.z + hash1010.w * dP0.w;
        real_t grad0110 = hash0110.x * dP0.x + hash0110.y * dP1.y + hash0110.z * dP1.z + hash0110.w * dP0.w;
        real_t grad1110 = hash1110.x * dP1.x + hash1110.y * dP1.y + hash1110.z * dP1.z + hash1110.w * dP0.w;
        real_t grad0001 = hash0001.x * dP0.x + hash0001.y * dP0.y + hash0001.z * dP0.z + hash0001.w * dP1.w;
        real_t grad1001 = hash1001.x * dP1.x + hash1001.y * dP0.y + hash1001.z * dP0.z + hash1001.w * dP1.w;
        real_t grad0101 = hash0101.x * dP0.x + hash0101.y * dP1.y + hash0101.z * dP0.z + hash0101.w * dP1.w;
        real_t grad1101 = hash1101.x * dP1.x + hash1101.y * dP1.y + hash1101.z * dP0.z + hash1101.w * dP1.w;
        real_t grad0011 = hash0011.x * dP0.x + hash0011.y * dP0.y + hash0011.z * dP1.z + hash0011.w * dP1.w;
        real_t grad1011 = hash1011.x * dP1.x + hash1011.y * dP0.y + hash1011.z * dP1.z + hash1011.w * dP1.w;
        real_t grad0111 = hash0111.x * dP0.x + hash0111.y * dP1.y + hash0111.z * dP1.z + hash0111.w * dP1.w;
        real_t grad1111 = hash1111.x * dP1.x + hash1111.y * dP1.y + hash1111.z * dP1.z + hash1111.w * dP1.w;

        real_t grad_yzw000 = grad0000 + (grad1000 - grad0000) * s.x;
        real_t grad_yzw100 = grad0100 + (grad1100 - grad0100) * s.x;
        real_t grad_yzw010 = grad0010 + (grad1010 - grad0010) * s.x;
        real_t grad_yzw110 = grad0110 + (grad1110 - grad0110) * s.x;
        real_t grad_yzw001 = grad0001 + (grad1001 - grad0001) * s.x;
        real_t grad_yzw101 = grad0101 + (grad1101 - grad0101) * s.x;
        real_t grad_yzw011 = grad0011 + (grad1011 - grad0011) * s.x;
        real_t grad_yzw111 = grad0111 + (grad1111 - grad0111) * s.x;

        real_t grad_zw00 = grad_yzw000 + (grad_yzw100 - grad_yzw000) * s.y;
        real_t grad_zw10 = grad_yzw010 + (grad_yzw110 - grad_yzw010) * s.y;
        real_t grad_zw01 = grad_yzw001 + (grad_yzw101 - grad_yzw001) * s.y;
        real_t grad_zw11 = grad_yzw011 + (grad_yzw111 - grad_yzw011) * s.y;

        real_t grad_w0 = grad_zw00 + (grad_zw10 - grad_zw00) * s.z;
        real_t grad_w1 = grad_zw01 + (grad_zw11 - grad_zw01) * s.z;

        real_t g = grad_w0 + (grad_w1 - grad_w0) * s.w;

        return GRADIENT_4D_NORMALIZATION_FACTOR * g;
    }

    template<typename real_t> real_t gradient_fbm4(const glm::tvec4<real_t>& P, const glm::tmat4x4<real_t>* matrices, const glm::tvec4<real_t>* shifts, const real_t* amplitudes)
    {
        real_t v = 0.0;
        for(int i = 0; i < 4; ++i)
        {
            glm::tvec4<real_t> PP = shifts[i] + matrices[i] * P;
            v += amplitudes[i] * gradient<real_t>(PP);
        }
        return v;
    }

    //===================================================================================================================================================================================================================
    // instantiate above template functions for real_t = float and real_t = double
    //===================================================================================================================================================================================================================

    template float  gradient<float> (const glm::vec4& P);
    template double gradient<double>(const glm::dvec4& P);
    template float  gradient<float> (const glm::vec2& P);
    template double gradient<double>(const glm::dvec2& P);
    template float  gradient<float> (const glm::vec3& P);
    template double gradient<double>(const glm::dvec3& P);

    template float  gradient_fbm4<float> (const glm::vec2&,  const glm::mat2*,  const glm::vec2*,  const float*);
    template double gradient_fbm4<double>(const glm::dvec2&, const glm::dmat2*, const glm::dvec2*, const double*);
    template float  gradient_fbm4<float> (const glm::vec3&,  const glm::mat3*,  const glm::vec3*,  const float*);
    template double gradient_fbm4<double>(const glm::dvec3&, const glm::dmat3*, const glm::dvec3*, const double*);
    template float  gradient_fbm4<float> (const glm::vec4&,  const glm::mat4*,  const glm::vec4*,  const float*);
    template double gradient_fbm4<double>(const glm::dvec4&, const glm::dmat4*, const glm::dvec4*, const double*);
}
