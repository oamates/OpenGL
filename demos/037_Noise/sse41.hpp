#ifndef _sse41_noise_included_4182735671253643872516257612031537856128035715235
#define _sse41_noise_included_4182735671253643872516257612031537856128035715235

#define GLM_FORCE_INLINE
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>

namespace noise {

    //===================================================================================================================================================================================================================
    // SSE41 implementation of value noise
    //===================================================================================================================================================================================================================
    template<typename real_t> real_t value_sse41(const glm::tvec2<real_t>& P);
    template<typename real_t> real_t value_sse41(const glm::tvec3<real_t>& P);
    template<typename real_t> real_t value_sse41(const glm::tvec4<real_t>& P);

    //===================================================================================================================================================================================================================
    // SSE41 implementation of simplex noise
    //===================================================================================================================================================================================================================
    template<typename real_t> real_t simplex_sse41(const glm::tvec2<real_t>& P);
    template<typename real_t> real_t simplex_sse41(const glm::tvec3<real_t>& P);
    template<typename real_t> real_t simplex_sse41(const glm::tvec4<real_t>& P);

    //===================================================================================================================================================================================================================
    // SSE41 implementation of gradient noise
    //===================================================================================================================================================================================================================
    template<typename real_t> real_t gradient_sse41(const glm::tvec2<real_t>& P);
    template<typename real_t> real_t gradient_sse41(const glm::tvec3<real_t>& P);
    template<typename real_t> real_t gradient_sse41(const glm::tvec4<real_t>& P);

} // namespace noise

#endif // _sse41_noise_included_4182735671253643872516257612031537856128035715235