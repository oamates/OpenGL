#ifndef _cms_range_included_879127560376561082035617354682375423754626723546827
#define _cms_range_included_879127560376561082035617354682375423754626723546827

#include <cassert>

namespace cms
{

struct Range
{
    float m_lower;
    float m_upper;

    Range() {}

    Range(float i_lower, float i_upper)
        : m_lower(i_lower), m_upper(i_upper)
    {
        assert(m_lower <= m_upper);
    }

};

} // namespace cms

#endif // _cms_range_included_879127560376561082035617354682375423754626723546827