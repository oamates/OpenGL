#include "value.hpp"
#include "common.hpp"

namespace noise {

    //===================================================================================================================================================================================================================
    // 2d value noise
    //===================================================================================================================================================================================================================
    template<typename real_t> real_t value(const glm::tvec2<real_t>& P)
    {
        glm::ivec2 Pi = entier(P);
        glm::tvec2<real_t> s = smooth_step5(P - glm::tvec2<real_t>(Pi));

        int32_t hash00 = HASH_FACTOR_X * Pi.x + HASH_FACTOR_Y * Pi.y;
        int32_t hash10 = hash00 + HASH_FACTOR_X;
        int32_t hash01 = hash00 + HASH_FACTOR_Y;
        int32_t hash11 = hash10 + HASH_FACTOR_Y;

        real_t val00 = real_t(hash(hash00));
        real_t val10 = real_t(hash(hash10));
        real_t val01 = real_t(hash(hash01));
        real_t val11 = real_t(hash(hash11));
        
        real_t val_y0 = val00 + (val10 - val00) * s.x;
        real_t val_y1 = val01 + (val11 - val01) * s.x;

        real_t val = val_y0 + (val_y1 - val_y0) * s.y;

        return real_t(VALUE_2D_NORMALIZATION_FACTOR) * val;
    }

    template<typename real_t> real_t value_fbm4(const glm::tvec2<real_t>& P, const glm::tmat2x2<real_t>* matrices, const glm::tvec2<real_t>* shifts, const real_t* amplitudes)
    {
        real_t v = 0.0;
        for(int i = 0; i < 4; ++i)
        {
            glm::tvec2<real_t> PP = shifts[i] + matrices[i] * P;
            v += amplitudes[i] * value<real_t>(PP);
        }
        return v;
    }

    //===================================================================================================================================================================================================================
    // 3d value noise
    //===================================================================================================================================================================================================================
    template<typename real_t> real_t value(const glm::tvec3<real_t>& P)
    {
        glm::ivec3 Pi = entier(P);
        glm::tvec3<real_t> s = smooth_step5(P - glm::tvec3<real_t>(Pi));

        int32_t hash000 = HASH_FACTOR_X * Pi.x + HASH_FACTOR_Y * Pi.y + HASH_FACTOR_Z * Pi.z;
        int32_t hash100 = hash000 + HASH_FACTOR_X;
        int32_t hash010 = hash000 + HASH_FACTOR_Y;
        int32_t hash110 = hash100 + HASH_FACTOR_Y;
        int32_t hash001 = hash000 + HASH_FACTOR_Z;
        int32_t hash101 = hash100 + HASH_FACTOR_Z;
        int32_t hash011 = hash010 + HASH_FACTOR_Z;
        int32_t hash111 = hash110 + HASH_FACTOR_Z;

        real_t val000 = real_t(hash(hash000));
        real_t val100 = real_t(hash(hash100));
        real_t val010 = real_t(hash(hash010));
        real_t val110 = real_t(hash(hash110));
        real_t val001 = real_t(hash(hash001));
        real_t val101 = real_t(hash(hash101));
        real_t val011 = real_t(hash(hash011));
        real_t val111 = real_t(hash(hash111));

        real_t val_yz00 = val000 + (val100 - val000) * s.x;
        real_t val_yz10 = val010 + (val110 - val010) * s.x;
        real_t val_yz01 = val001 + (val101 - val001) * s.x;
        real_t val_yz11 = val011 + (val111 - val011) * s.x;

        real_t val_z0 = val_yz00 + (val_yz10 - val_yz00) * s.y;
        real_t val_z1 = val_yz01 + (val_yz11 - val_yz01) * s.y;

        real_t val = val_z0 + (val_z1 - val_z0) * s.z;

        return real_t(VALUE_3D_NORMALIZATION_FACTOR) * val;
    }

    template<typename real_t> real_t value_fbm4(const glm::tvec3<real_t>& P, const glm::tmat3x3<real_t>* matrices, const glm::tvec3<real_t>* shifts, const real_t* amplitudes)
    {
        real_t v = 0.0;
        for(int i = 0; i < 4; ++i)
        {
            glm::tvec3<real_t> PP = shifts[i] + matrices[i] * P;
            v += amplitudes[i] * value<real_t>(PP);
        }
        return v;
    }

    //===================================================================================================================================================================================================================
    // 4d value noise
    //===================================================================================================================================================================================================================
    template<typename real_t> real_t value(const glm::tvec4<real_t>& P)
    {
        glm::ivec4 Pi = entier(P);
        glm::tvec4<real_t> s = smooth_step5(P - glm::tvec4<real_t>(Pi));

        int32_t hash0000 = HASH_FACTOR_X * Pi.x + HASH_FACTOR_Y * Pi.y + HASH_FACTOR_Z * Pi.z + HASH_FACTOR_W * Pi.w;
        int32_t hash1000 = hash0000 + HASH_FACTOR_X;
        int32_t hash0100 = hash0000 + HASH_FACTOR_Y;
        int32_t hash1100 = hash1000 + HASH_FACTOR_Y;
        int32_t hash0010 = hash0000 + HASH_FACTOR_Z;
        int32_t hash1010 = hash1000 + HASH_FACTOR_Z;
        int32_t hash0110 = hash0100 + HASH_FACTOR_Z;
        int32_t hash1110 = hash1100 + HASH_FACTOR_Z;
        int32_t hash0001 = hash0000 + HASH_FACTOR_W;
        int32_t hash1001 = hash1000 + HASH_FACTOR_W;
        int32_t hash0101 = hash0100 + HASH_FACTOR_W;
        int32_t hash1101 = hash1100 + HASH_FACTOR_W;
        int32_t hash0011 = hash0010 + HASH_FACTOR_W;
        int32_t hash1011 = hash1010 + HASH_FACTOR_W;
        int32_t hash0111 = hash0110 + HASH_FACTOR_W;
        int32_t hash1111 = hash1110 + HASH_FACTOR_W;

        real_t val0000 = real_t(hash(hash0000));
        real_t val1000 = real_t(hash(hash1000));
        real_t val0100 = real_t(hash(hash0100));
        real_t val1100 = real_t(hash(hash1100));
        real_t val0010 = real_t(hash(hash0010));
        real_t val1010 = real_t(hash(hash1010));
        real_t val0110 = real_t(hash(hash0110));
        real_t val1110 = real_t(hash(hash1110));
        real_t val0001 = real_t(hash(hash0001));
        real_t val1001 = real_t(hash(hash1001));
        real_t val0101 = real_t(hash(hash0101));
        real_t val1101 = real_t(hash(hash1101));
        real_t val0011 = real_t(hash(hash0011));
        real_t val1011 = real_t(hash(hash1011));
        real_t val0111 = real_t(hash(hash0111));
        real_t val1111 = real_t(hash(hash1111));

        real_t val_yzw000 = val0000 + (val1000 - val0000) * s.x;
        real_t val_yzw100 = val0100 + (val1100 - val0100) * s.x;
        real_t val_yzw010 = val0010 + (val1010 - val0010) * s.x;
        real_t val_yzw110 = val0110 + (val1110 - val0110) * s.x;
        real_t val_yzw001 = val0001 + (val1001 - val0001) * s.x;
        real_t val_yzw101 = val0101 + (val1101 - val0101) * s.x;
        real_t val_yzw011 = val0011 + (val1011 - val0011) * s.x;
        real_t val_yzw111 = val0111 + (val1111 - val0111) * s.x;

        real_t val_zw00 = val_yzw000 + (val_yzw100 - val_yzw000) * s.y;
        real_t val_zw10 = val_yzw010 + (val_yzw110 - val_yzw010) * s.y;
        real_t val_zw01 = val_yzw001 + (val_yzw101 - val_yzw001) * s.y;
        real_t val_zw11 = val_yzw011 + (val_yzw111 - val_yzw011) * s.y;

        real_t val_w0 = val_zw00 + (val_zw10 - val_zw00) * s.z;
        real_t val_w1 = val_zw01 + (val_zw11 - val_zw01) * s.z;

        real_t val = val_w0 + (val_w1 - val_w0) * s.w;

        return real_t(VALUE_4D_NORMALIZATION_FACTOR) * val;
    }    

    template<typename real_t> real_t value_fbm4(const glm::tvec4<real_t>& P, const glm::tmat4x4<real_t>* matrices, const glm::tvec4<real_t>* shifts, const real_t* amplitudes)
    {
        real_t v = 0.0;
        for(int i = 0; i < 4; ++i)
        {
            glm::tvec4<real_t> PP = shifts[i] + matrices[i] * P;
            v += amplitudes[i] * value<real_t>(PP);
        }
        return v;
    }

    //===================================================================================================================================================================================================================
    // instantiate above template functions for real_t = float and real_t = double
    //===================================================================================================================================================================================================================

    template float  value<float> (const glm::vec2&);
    template double value<double>(const glm::dvec2&);
    template float  value<float> (const glm::vec3&);
    template double value<double>(const glm::dvec3&);
    template float  value<float> (const glm::vec4&);
    template double value<double>(const glm::dvec4&);


    template float  value_fbm4<float> (const glm::vec2&,  const glm::mat2*,  const glm::vec2*,  const float*);
    template double value_fbm4<double>(const glm::dvec2&, const glm::dmat2*, const glm::dvec2*, const double*);
    template float  value_fbm4<float> (const glm::vec3&,  const glm::mat3*,  const glm::vec3*,  const float*);
    template double value_fbm4<double>(const glm::dvec3&, const glm::dmat3*, const glm::dvec3*, const double*);
    template float  value_fbm4<float> (const glm::vec4&,  const glm::mat4*,  const glm::vec4*,  const float*);
    template double value_fbm4<double>(const glm::dvec4&, const glm::dmat4*, const glm::dvec4*, const double*);

}
