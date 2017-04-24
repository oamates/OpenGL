#ifndef TRI_STRIPPER_HEADER_GUARD_POLICY_H
#define TRI_STRIPPER_HEADER_GUARD_POLICY_H

#include "../public_types.h"
#include "types.h"

namespace triangle_stripper {
namespace detail {

struct policy
{
    policy(size_t MinStripSize, bool Cache) : m_Degree(0), m_CacheHits(0), m_MinStripSize(MinStripSize), m_Cache(Cache) { }

    strip BestStrip() const
        { return m_Strip; }
    void Challenge(strip Strip, size_t Degree, size_t CacheHits);

    strip   m_Strip;
    size_t  m_Degree;
    size_t  m_CacheHits;
    size_t  m_MinStripSize;
    bool    m_Cache;
};

} // namespace detail
} // namespace triangle_stripper

#endif // TRI_STRIPPER_HEADER_GUARD_POLICY_H
