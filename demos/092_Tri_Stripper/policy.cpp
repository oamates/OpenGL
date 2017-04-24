#include "detail/policy.h"

namespace triangle_stripper {
namespace detail {

void policy::Challenge(strip Strip, size_t Degree, size_t CacheHits)
{
    if (Strip.Size() < m_MinStripSize) return;
    if (!m_Cache)                                                                       // Cache is disabled, take the longest strip
    {
        if (Strip.Size() > m_Strip.Size()) m_Strip = Strip;                             // Cache simulator enabled
    }
    else
    {
        if (CacheHits > m_CacheHits)                                                    // Priority 1: Keep the strip with the best cache hit count
        {
            m_Strip = Strip;
            m_Degree = Degree;
            m_CacheHits = CacheHits;
        }
        else if (CacheHits == m_CacheHits)
        {
            if ((m_Strip.Size() != 0) && (Degree < m_Degree))                           // Priority 2: Keep the strip with the loneliest start triangle
            {
                m_Strip = Strip;
                m_Degree = Degree;
            }
            else if (Strip.Size() > m_Strip.Size())                                     // Priority 3: Keep the longest strip 
            {
                m_Strip = Strip;
                m_Degree = Degree;
            }
        }
    }
}

} // namespace detail
} // namespace triangle_stripper
