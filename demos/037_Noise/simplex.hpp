#ifndef _simplex_noise_included_58923459687463753465716708563825610865086512531
#define _simplex_noise_included_58923459687463753465716708563825610865086512531

#define GLM_FORCE_INLINE
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>

namespace noise {

    //===================================================================================================================================================================================================================
    // Simplex noise :: a hash value on a 2d-, 3d- or 4d-lattice point is obtained by
    //  1. taking the value of a linear as an initial pre-hash value
    //  2. XOR with right shift is used to randomize its lower bits
    //  3. multiplication is used to randomize higher bits
    //===================================================================================================================================================================================================================

    //===================================================================================================================================================================================================================
    // 2d, 3d and 4d simplex noise for vectors with float/double components - default implementation
    //===================================================================================================================================================================================================================
    template<typename real_t> real_t simplex(const glm::tvec2<real_t>& P);
    template<typename real_t> real_t simplex(const glm::tvec3<real_t>& P);
    template<typename real_t> real_t simplex(const glm::tvec4<real_t>& P);

    template<typename real_t> real_t simplex_fbm4(const glm::tvec2<real_t>& P, const glm::tmat2x2<real_t>* matrices, const glm::tvec2<real_t>* shifts, const real_t* amplitudes);
    template<typename real_t> real_t simplex_fbm4(const glm::tvec3<real_t>& P, const glm::tmat3x3<real_t>* matrices, const glm::tvec3<real_t>* shifts, const real_t* amplitudes);
    template<typename real_t> real_t simplex_fbm4(const glm::tvec4<real_t>& P, const glm::tmat4x4<real_t>* matrices, const glm::tvec4<real_t>* shifts, const real_t* amplitudes);

    //===================================================================================================================================================================================================================
    // 2d, 3d and 4d simplex noise for vectors with float/double components - SSE2 implementation
    //===================================================================================================================================================================================================================
/*    template<typename real_t> real_t value(const glm::tvec2<real_t>& P);
    template<typename real_t> real_t value(const glm::tvec3<real_t>& P);
    template<typename real_t> real_t value(const glm::tvec4<real_t>& P); */

    //===================================================================================================================================================================================================================
    // 2d, 3d and 4d simplex noise for vectors with float/double components - SSE41 implementation
    //===================================================================================================================================================================================================================
/*    template<typename real_t> real_t value(const glm::tvec2<real_t>& P);
    template<typename real_t> real_t value(const glm::tvec3<real_t>& P);
    template<typename real_t> real_t value(const glm::tvec4<real_t>& P); */

    //===================================================================================================================================================================================================================
    // 2d, 3d and 4d simplex noise for vectors with float/double components - AVX2 implementation
    //===================================================================================================================================================================================================================
/*    template<typename real_t> real_t value(const glm::tvec2<real_t>& P);
    template<typename real_t> real_t value(const glm::tvec3<real_t>& P);
    template<typename real_t> real_t value(const glm::tvec4<real_t>& P); */

} // namespace noise

#endif // _simplex_noise_included_58923459687463753465716708563825610865086512531