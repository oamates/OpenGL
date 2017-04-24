#ifndef _sse2_noise_included_81759637454134510348513469865143987516357613476513
#define _sse2_noise_included_81759637454134510348513469865143987516357613476513

#define GLM_FORCE_INLINE
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>

namespace noise {

    //===================================================================================================================================================================================================================
    // SSE2 implementation of value noise
    //===================================================================================================================================================================================================================
    template<typename real_t> real_t value_sse2(const glm::tvec2<real_t>& P);
    template<typename real_t> real_t value_sse2(const glm::tvec3<real_t>& P);
    template<typename real_t> real_t value_sse2(const glm::tvec4<real_t>& P);

    //===================================================================================================================================================================================================================
    // SSE2 implementation of simplex noise
    //===================================================================================================================================================================================================================
    template<typename real_t> real_t simplex_sse2(const glm::tvec2<real_t>& P);
    template<typename real_t> real_t simplex_sse2(const glm::tvec3<real_t>& P);
    template<typename real_t> real_t simplex_sse2(const glm::tvec4<real_t>& P);

    //===================================================================================================================================================================================================================
    // SSE2 implementation of gradient noise
    //===================================================================================================================================================================================================================
    template<typename real_t> real_t gradient_sse2(const glm::tvec2<real_t>& P);
    template<typename real_t> real_t gradient_sse2(const glm::tvec3<real_t>& P);
    template<typename real_t> real_t gradient_sse2(const glm::tvec4<real_t>& P);

} // namespace noise

#endif // _sse2_noise_included_81759637454134510348513469865143987516357613476513