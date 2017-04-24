#ifndef _avx2_noise_included_81759634751934658713465138745683746518734651873465
#define _avx2_noise_included_81759634751934658713465138745683746518734651873465

#define GLM_FORCE_INLINE
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>

namespace noise {

    //===================================================================================================================================================================================================================
    // AVX 2 implementation of value noise
    //===================================================================================================================================================================================================================
    template<typename real_t> real_t value_avx2(const glm::tvec2<real_t>& P);
    template<typename real_t> real_t value_avx2(const glm::tvec3<real_t>& P);
    template<typename real_t> real_t value_avx2(const glm::tvec4<real_t>& P);

    double value_fbm4_avx2(const glm::dvec2& point, const glm::dmat2* matrices, const glm::dvec2* shifts, const glm::dvec4& amplitudes);


    //===================================================================================================================================================================================================================
    // AVX 2 implementation of simplex noise
    //===================================================================================================================================================================================================================
    template<typename real_t> real_t simplex_avx2(const glm::tvec2<real_t>& P);
    template<typename real_t> real_t simplex_avx2(const glm::tvec3<real_t>& P);
    template<typename real_t> real_t simplex_avx2(const glm::tvec4<real_t>& P);

    //===================================================================================================================================================================================================================
    // AVX 2 implementation of gradient noise
    //===================================================================================================================================================================================================================
    template<typename real_t> real_t gradient_avx2(const glm::tvec2<real_t>& P);
    template<typename real_t> real_t gradient_avx2(const glm::tvec3<real_t>& P);
    template<typename real_t> real_t gradient_avx2(const glm::tvec4<real_t>& P);

} // namespace noise

#endif // _avx2_noise_included_81759634751934658713465138745683746518734651873465