#ifndef _gradient_noise_included_2536348571238957612409756120935760124978569274
#define _gradient_noise_included_2536348571238957612409756120935760124978569274

#define GLM_FORCE_INLINE
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>

namespace noise {

    //===================================================================================================================================================================================================================
    // 3d gradient noise :: a hash value on a 2d-, 3d- or 4d-lattice point is obtained by
    //  1. taking the value of a linear as an initial pre-hash value
    //  2. XOR with right shift is used to randomize its lower bits
    //  3. multiplication is used to randomize higher bits
    //===================================================================================================================================================================================================================

    //===================================================================================================================================================================================================================
    // 2d, 3d and 4d gradient noise for vectors with float/double components - default implementation
    //===================================================================================================================================================================================================================
    template<typename real_t> real_t gradient(const glm::tvec2<real_t>& P);
    template<typename real_t> real_t gradient(const glm::tvec3<real_t>& P);
    template<typename real_t> real_t gradient(const glm::tvec4<real_t>& P);

    template<typename real_t> real_t gradient_fbm4(const glm::tvec2<real_t>& P, const glm::tmat2x2<real_t>* matrices, const glm::tvec2<real_t>* shifts, const real_t* amplitudes);
    template<typename real_t> real_t gradient_fbm4(const glm::tvec3<real_t>& P, const glm::tmat3x3<real_t>* matrices, const glm::tvec3<real_t>* shifts, const real_t* amplitudes);
    template<typename real_t> real_t gradient_fbm4(const glm::tvec4<real_t>& P, const glm::tmat4x4<real_t>* matrices, const glm::tvec4<real_t>* shifts, const real_t* amplitudes);

    //===================================================================================================================================================================================================================
    // 2d, 3d and 4d gradient noise for vectors with float/double components - SSE2 implementation
    //===================================================================================================================================================================================================================
/*    template<typename real_t> real_t gradient(const glm::tvec2<real_t>& P);
    template<typename real_t> real_t gradient(const glm::tvec3<real_t>& P);
    template<typename real_t> real_t gradient(const glm::tvec4<real_t>& P); */

    //===================================================================================================================================================================================================================
    // 2d, 3d and 4d gradient noise for vectors with float/double components - SSE41 implementation
    //===================================================================================================================================================================================================================
/*    template<typename real_t> real_t gradient(const glm::tvec2<real_t>& P);
    template<typename real_t> real_t gradient(const glm::tvec3<real_t>& P);
    template<typename real_t> real_t gradient(const glm::tvec4<real_t>& P); */

    //===================================================================================================================================================================================================================
    // 2d, 3d and 4d gradient noise for vectors with float/double components - AVX2 implementation
    //===================================================================================================================================================================================================================
/*    template<typename real_t> real_t gradient(const glm::tvec2<real_t>& P);
    template<typename real_t> real_t gradient(const glm::tvec3<real_t>& P);
    template<typename real_t> real_t gradient(const glm::tvec4<real_t>& P); */
}

#endif // _gradient_noise_included_2536348571238957612409756120935760124978569274