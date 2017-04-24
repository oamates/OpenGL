#ifndef _value_noise_included_8741346078634516304739048726348796278568374483753
#define _value_noise_included_8741346078634516304739048726348796278568374483753

#define GLM_FORCE_INLINE
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>

namespace noise {

    //===================================================================================================================================================================================================================
    // 2d, 3d and 4d value noise for vectors with float/double components - default implementation
    //===================================================================================================================================================================================================================
    template<typename real_t> real_t value(const glm::tvec2<real_t>& P);
    template<typename real_t> real_t value(const glm::tvec3<real_t>& P);
    template<typename real_t> real_t value(const glm::tvec4<real_t>& P);

    template<typename real_t> real_t value_fbm4(const glm::tvec2<real_t>& P, const glm::tmat2x2<real_t>* matrices, const glm::tvec2<real_t>* shifts, const real_t* amplitudes);
    template<typename real_t> real_t value_fbm4(const glm::tvec3<real_t>& P, const glm::tmat3x3<real_t>* matrices, const glm::tvec3<real_t>* shifts, const real_t* amplitudes);
    template<typename real_t> real_t value_fbm4(const glm::tvec4<real_t>& P, const glm::tmat4x4<real_t>* matrices, const glm::tvec4<real_t>* shifts, const real_t* amplitudes);


    //===================================================================================================================================================================================================================
    // 2d, 3d and 4d value noise for vectors with float/double components - SSE2 implementation
    //===================================================================================================================================================================================================================
/*    template<typename real_t> real_t value(const glm::tvec2<real_t>& P);
    template<typename real_t> real_t value(const glm::tvec3<real_t>& P);
    template<typename real_t> real_t value(const glm::tvec4<real_t>& P); */

    //===================================================================================================================================================================================================================
    // 2d, 3d and 4d value noise for vectors with float/double components - SSE41 implementation
    //===================================================================================================================================================================================================================
/*    template<typename real_t> real_t value(const glm::tvec2<real_t>& P);
    template<typename real_t> real_t value(const glm::tvec3<real_t>& P);
    template<typename real_t> real_t value(const glm::tvec4<real_t>& P); */

    //===================================================================================================================================================================================================================
    // 2d, 3d and 4d value noise for vectors with float/double components - AVX2 implementation
    //===================================================================================================================================================================================================================
/*    template<typename real_t> real_t value(const glm::tvec2<real_t>& P);
    template<typename real_t> real_t value(const glm::tvec3<real_t>& P);
    template<typename real_t> real_t value(const glm::tvec4<real_t>& P); */

} // namespace noise

#endif // _value_noise_included_8741346078634516304739048726348796278568374483753